#include "mlpbf/character.h"
#include "mlpbf/console.h"
#include "mlpbf/database.h"
#include "mlpbf/direction.h"
#include "mlpbf/exception.h"
#include "mlpbf/farm.h"
#include "mlpbf/global.h"
#include "mlpbf/map.h"
#include "mlpbf/resource.h"
#include "mlpbf/time/season.h"

#include <algorithm>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <sstream>

namespace bf
{

inline void assertBounds( const sf::Vector2u& pos, unsigned width, unsigned height )
{
	if ( pos.x < 0 || width <= pos.x )
		throw Exception( "x-position is out of map bounds" );
	if ( pos.y < 0 || height <= pos.y )
		throw Exception( "y-position is out of map bounds" );
}

inline void renderLayer( sf::RenderTarget& target, sf::RenderStates& states, 
						 const Map& map, const std::vector< const Tmx::Layer* >& layer, 
						 const sf::FloatRect& rect, const sf::IntRect& draw )
{
	sf::Sprite sprite;
	for ( auto it = layer.begin(); it != layer.end(); ++it )
		for ( int y = std::max( draw.top, 0 ); y < draw.top + draw.height; y++ )
			for ( int x = std::max( draw.left, 0 ); x < draw.left + draw.width; x++ )
			{
				if ( map.adjustSprite( **it, sf::Vector2u( x, y ), sprite ) )
				{
					sprite.move( -rect.left, -rect.top );
					target.draw( sprite, states );
				}
			}
}

inline float round( float f )
{
	if ( f - std::floor( f ) >= 0.5f )
		return std::ceil( f );
	return std::floor( f );
}

inline sf::Vector2f round( const sf::Vector2f& f )
{
	return sf::Vector2f( round( f.x ), round( f.y ) );
}

inline void setNeighbor( const std::map< std::string, std::string >& map, const std::string& str, std::pair< bf::Map*, int >& m )
{
	auto find = map.find( str );
	if ( find != map.end() )
	{
		std::string::size_type pos = find->second.find( ',' );
		std::string _map = ( ( pos != std::string::npos ) ? find->second.substr( 0, pos ) : find->second );

		m.first = &db::getMap( _map );
		if ( pos != std::string::npos )
			std::istringstream( find->second.substr( pos + 1 ) ) >> m.second;
	}
}

/***************************************************************************/

class Map::Object : public sf::Drawable, private sf::Transformable
{
public:
	friend Map::Object * generateObject( const Tmx::Object & );
	virtual ~Object() {}

	const std::string & getName() const { return m_name; }
	const sf::FloatRect & getBounds() const { return m_bounds; }

	using sf::Transformable::getPosition;
	using sf::Transformable::setPosition;

public:
	virtual void load( const Tmx::Object& object ) = 0;
	virtual void update( sf::Uint32 frameTime, const sf::Vector2f& pos ) {}

	virtual void onEnter( sf::Uint32 frameTime, const sf::Vector2f& pos ) {}
	virtual void whileInside( sf::Uint32 frameTime, const sf::Vector2f& pos ) {}
	virtual void onExit( sf::Uint32 frameTime, const sf::Vector2f& pos ) {}

	virtual void onInteract( const sf::Vector2f& pos ) {};

	virtual bool hasCollision( const sf::Vector2f& pos ) const = 0;

protected:
	using sf::Transformable::getTransform;

private:
	std::string m_name;
	sf::FloatRect m_bounds;
};

Map::Object * generateObject( const Tmx::Object & tmxObject );

/***************************************************************************/

static Map* GLOBAL_MAP = nullptr;

Map& Map::global()
{
	if ( !GLOBAL_MAP )
		throw Exception( "No map loaded!" );
	return *GLOBAL_MAP;
}

Map& Map::global( unsigned id )
{
	return *( GLOBAL_MAP = &db::getMap( id ) );
}

Map& Map::global( const std::string& map )
{
	return *( GLOBAL_MAP = &db::getMap( map ) );
}

/***************************************************************************/

Map::~Map()
{
	for ( Map::Object * obj : m_objects )
		delete obj;
	m_objects.clear();
}

bool Map::adjustSprite( const Tmx::Layer& layer, sf::Vector2u pos, sf::Sprite& sprite ) const
{
	assertBounds( pos, getWidth(), getHeight() );

	const Tmx::MapTile& tile = layer.GetTile( pos.x, pos.y );
	if ( tile.tileset == nullptr ) return false;

	const Tmx::Tileset& tileset = *tile.tileset;

	const std::shared_ptr< sf::Texture >& texture = m_textures.find( &tileset )->second;
	unsigned tilesetWidth = texture->getSize().x / TILE_WIDTH;
	
	sf::IntRect rect;
	rect.left	= tile.id % tilesetWidth * TILE_WIDTH;
	rect.top	= tile.id / tilesetWidth * TILE_HEIGHT;
	rect.width	= TILE_WIDTH;
	rect.height = TILE_HEIGHT;

	sprite.setPosition( (float) pos.x * TILE_WIDTH, (float) pos.y * TILE_HEIGHT );
	sprite.setTexture( *texture );
	sprite.setTextureRect( rect );

	return true;
}

bool Map::checkTileCollision( const sf::Vector2u& pos ) const
{
	return m_collision && ( 0 <= pos.x && pos.x < getWidth() && 0 <= pos.y && pos.y < getHeight() ) && m_collision->GetTile( pos.x, pos.y ).tileset != 0;
}

bool Map::checkObjectCollision( const sf::Vector2f& pos ) const
{
	if ( !m_collision ) 
		return false;

	for ( auto it = m_objects.begin(); it != m_objects.end(); ++it )
	{
		const Map::Object& obj = **it;
		if ( obj.getBounds().contains( pos ) && obj.hasCollision( pos - obj.getPosition() ) )
			return true;
	}
	return false;
}

void Map::load( unsigned id, const std::string& map )
{
	m_mapID = id;
	m_map.ParseFile( map );

	if ( m_map.HasError() )
		throw Exception( m_map.GetErrorText().c_str() );

	m_collision = nullptr;
	std::fill( m_neighbors.begin(), m_neighbors.end(), std::make_pair( nullptr, 0 ) );

	// Load tilesets
	const auto& tilesets = m_map.GetTilesets();
	for ( auto it = tilesets.begin(); it != tilesets.end(); ++it )
	{
		const std::string& base = (*it)->GetSource();

		std::string file;
		if ( !base.empty() ) // If externally loaded, prepend the location minus the final '/'
			file = base.substr( 0, base.find_last_of( '/' ) + 1 );
		file += (*it)->GetImage()->GetSource();

		std::shared_ptr< sf::Texture > texture = res::loadTexture( file );
		m_textures.insert( std::make_pair( *it, texture ) );
	}

	// Load layers
	const auto& layers = m_map.GetLayers();
	for ( auto it = layers.begin(); it != layers.end(); ++it )
	{
		const auto& properties = (*it)->GetProperties().GetList();
		bool add = true, upper = false;

		if ( m_collision == nullptr )
		{
			std::string name = (*it)->GetName();
			std::transform( name.begin(), name.end(), name.begin(), ::tolower );
			if ( name == "collision" )
			{
				m_collision = *it;
				continue;
			}
		}

		auto findSeason = properties.find( "season" );
		if ( findSeason != properties.end() )
			add = time::parseSeasons( findSeason->second )[ m_season ];

		auto findRender = properties.find( "render" );
		if ( findRender != properties.end() )
			upper = ( findRender->second == "above" );

		if ( add ) 
			( upper ? m_upper : m_lower ).push_back( *it );
	}

	// Load objects
	const auto& objects = m_map.GetObjectGroups();
	for ( auto it = objects.begin(); it != objects.end(); ++it )
	{
		const auto& objectGroup = (*it)->GetObjects();
		for ( auto ij = objectGroup.begin(); ij != objectGroup.end(); ++ij )
		{
			const Tmx::Object& object = *(*ij);

			const std::string& name = object.GetName();
			const std::string& type = object.GetType();

			try
			{
				m_objects.push_back( generateObject( object ) );
			}
			catch ( std::exception& err )
			{
				Console::singleton() << con::setcerr << map << ": failed to load object \"" << name << "\": " << err.what() << con::endl;
			}
		}
	}

	if ( !m_collision )
		Console::singleton() << con::setcerr << "Warning: Map \"" << map << "\" does not have a collision layer!" << con::endl;
}

void Map::loadNeighbors()
{
	const auto& properties = m_map.GetProperties().GetList();

	setNeighbor( properties, "north", m_neighbors[ Up ] );
	setNeighbor( properties, "south", m_neighbors[ Down ] );
	setNeighbor( properties, "west", m_neighbors[ Left ] );
	setNeighbor( properties, "east", m_neighbors[ Right ] );
}

void Map::update( sf::Uint32 frameTime, const sf::Vector2f& pos )
{
	std::vector< Map::Object* > remove;

	// Check if the player has left any of the active objects
	for ( auto it = m_activeObjects.begin(); it != m_activeObjects.end(); ++it )
		if ( !(*it)->getBounds().contains( pos ) )
			remove.push_back( *it );

	// Remove the objects that the player has left and call their onExit
	for ( auto it = remove.begin(); it != remove.end(); ++it )
	{
		(*it)->onExit( frameTime, pos - (*it)->getPosition() );
		std::remove( m_activeObjects.begin(), m_activeObjects.end(), *it );
	}

	// Update all objects on the map
	for ( auto it = m_objects.begin(); it != m_objects.end(); ++it )
		(*it)->update( frameTime, pos - (*it)->getPosition() );

	// Update all active objects
	for ( auto it = m_activeObjects.begin(); it != m_activeObjects.end(); ++it )
		(*it)->whileInside( frameTime, pos - (*it)->getPosition() );

	// Check if the player has entered any new objects
	for ( auto it = m_objects.begin(); it != m_objects.end(); ++it )
	{
		const sf::FloatRect& rect = (*it)->getBounds();
		if ( rect.contains( pos ) && std::find( m_activeObjects.begin(), m_activeObjects.end(), *it ) != m_activeObjects.end() )
		{
			(*it)->onEnter( frameTime, pos - (*it)->getPosition() );
			m_activeObjects.push_back( *it );
		}
	}
}

bool Map::interact( const sf::Vector2f& pos )
{
	bool ret = false;
	for ( Map::Object * obj : m_objects )
		if ( obj->getBounds().contains( pos ) )
		{
			obj->onInteract( pos - obj->getPosition() );
			ret = true;
		}
	return ret;
}

/***************************************************************************/

MapViewer::MapViewer( const Map& map ) :
	m_map( &map ),
	m_area( 0.0f, 0.0f, (float) SCREEN_WIDTH, (float) SCREEN_HEIGHT )
{
}

void MapViewer::center( const sf::Vector2f& pos )
{
	sf::Vector2f p = round( pos );
	m_area.left = p.x - m_area.width  / 2.0f;
	m_area.top  = p.y - m_area.height / 2.0f;
}

const sf::Vector2f MapViewer::center() const
{
	return sf::Vector2f( m_area.left - m_area.width / 2.0f, m_area.top - m_area.height / 2.0f );
}

void MapViewer::dimension( const sf::Vector2f& dim )
{
	m_area.width = dim.x;
	m_area.height = dim.y;
}

void MapViewer::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	states.transform *= getTransform();

	sf::FloatRect rect = m_area;

	sf::IntRect draw;
	draw.left	= std::max( 0, static_cast< int >( rect.left / TILE_WIDTH ) );
	draw.top	= std::max( 0, static_cast< int >( rect.top / TILE_HEIGHT ) );
	draw.width	= static_cast< int >( std::ceil( rect.width / TILE_WIDTH ) + 1 );
	draw.height = static_cast< int >( std::ceil( rect.height / TILE_HEIGHT ) + 1 );

	if ( draw.left + draw.width >= (int) m_map->getWidth() )
		draw.left = m_map->getWidth() - draw.width;
	if ( draw.top + draw.height >= (int) m_map->getHeight() )
		draw.top = m_map->getHeight() - draw.height;

	// Render lower layer
	renderLayer( target, states, *m_map, m_map->getLowerLayers(), rect, draw );

	// Draw objects -- WARNING: UGLY CODE
	const auto& objects = m_map->getObjects();
	for ( auto it = objects.begin(); it != objects.end(); ++it )
	{
		const sf::FloatRect& objRect = (*it)->getBounds();
		if ( rect.intersects( objRect ) )
		{
			Map::Object& object = const_cast< Map::Object& >( **it );
			object.setPosition( objRect.left - rect.left, objRect.top - rect.top );

			target.draw( **it, states );

			// Reset it
			object.setPosition( objRect.left, objRect.top );
		}
	}

	// Render character(s)
	for ( auto it = m_characters.begin(); it != m_characters.end(); ++it )
	{
		const Character& c = **it;
		if ( c.getMapID() == m_map->getID() && rect.intersects( c.getBounds() ) )
		{
			sf::Sprite sprite = c.toSprite();
			sprite.move( -rect.left, -rect.top );
			target.draw( sprite, states );

			if ( DEBUG_COLLISION )
			{
				sf::FloatRect bounds = c.getBounds();

				sf::RectangleShape col;
				col.setPosition( bounds.left - rect.left, bounds.top - rect.top );
				col.setSize( sf::Vector2f( bounds.width, bounds.height ) );
				col.setFillColor( sf::Color( 200, 0, 0, 150 ) );

				target.draw( col, states );
			}
		}
	}
			
	// Render upper layer
	renderLayer( target, states, *m_map, m_map->getUpperLayers(), rect, draw );

	// Optional: Render the collision layer
	if ( DEBUG_COLLISION && m_map->getCollisionLayer() )
	{
		std::vector< const Tmx::Layer* > t;
		t.push_back( m_map->getCollisionLayer() );
		renderLayer( target, states, *m_map, t, rect, draw );
	}
}

/***************************************************************************/

void MultiMapViewer::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	const Map& m = map();
	const sf::FloatRect& area = getViewArea();
	sf::Vector2f center = this->center(), offset;

	MapViewer child( *this );

	// Draw west map
	if ( m.getNeighbor( Left ) &&  area.left < 0 )
	{
		offset.x = m.getNeighbor( Left )->getWidth() * TILE_WIDTH + area.left + ( area.width / 2.0f );
		offset.y = ( area.top + area.height / 2.0f ) + ( m.getNeighborOffset( Left ) * TILE_HEIGHT );

		child.map( *m.getNeighbor( Left ) );
		child.center( offset );
		child.dimension( sf::Vector2f( -area.left, SCREEN_HEIGHT ) );

		target.draw( child, states );
	}
	
	// Draw east map
	if ( m.getNeighbor( Right ) && m.getWidth() * TILE_WIDTH <= area.left + area.width )
	{
		offset.x = ( area.left + area.width ) - ( m.getWidth() * TILE_WIDTH );
		offset.y = ( area.top + area.height / 2.0f ) + ( m.getNeighborOffset( Right ) * TILE_HEIGHT );

		child.map( *m.getNeighbor( Right ) );
		child.center( offset );
		child.setPosition( SCREEN_WIDTH - offset.x + child.getViewArea().left, 0.0f );
		child.dimension( sf::Vector2f( SCREEN_WIDTH - child.getPosition().x, SCREEN_HEIGHT ) );

		target.draw( child, states );
	}

	// Draw north map (copy of west, x changed to y and width changed to height)
	if ( m.getNeighbor( Up ) && area.top < 0 )
	{
		offset.x = ( area.left + area.width / 2.0f ) + ( m.getNeighborOffset( Up ) * TILE_WIDTH );
		offset.y = m.getNeighbor( Up )->getHeight() * TILE_HEIGHT + area.top + ( area.height / 2.0f );

		child.map( *m.getNeighbor( Up ) );
		child.center( offset );
		child.dimension( sf::Vector2f( SCREEN_WIDTH, -area.top ) );

		target.draw( child, states );
	}

	// Draw south map (copy of east, x changed to y and width changed to height)
	if ( m.getNeighbor( Down ) && m.getHeight() * TILE_HEIGHT <= area.top + area.height )
	{
		offset.x = ( area.left + area.width / 2.0f ) + ( m.getNeighborOffset( Up ) * TILE_WIDTH );
		offset.y = ( area.top + area.height ) - ( m.getHeight() * TILE_HEIGHT );

		child.map( *m.getNeighbor( Down ) );
		child.center( offset );
		child.setPosition( 0.0f, SCREEN_HEIGHT - offset.y + child.getViewArea().top );
		child.dimension( sf::Vector2f( SCREEN_WIDTH, SCREEN_HEIGHT - child.getPosition().y ) );

		target.draw( child, states );
	}

	// Draw the original map
	MapViewer::draw( target, states );	
}

/***************************************************************************/

//-------------------------------------------------------------------------
// A map object is an abstract class for an object that appears on a map
// The object must implement methods when the player interacts with the object in several ways
// 
// FUNCTION EXPLANATIONS
//
//		const sf::FloatRect& getBounds() const
//			Returns the FloatRect of the object's bounds
//			NOTE: left != getPosition().x and top != getPosition().y
//
//	IMPLEMENTABLE METHODS
//
//		void load( const Tmx::Object&, Instance&, Map& ) [pure virtual]
//			Called once when the object is being created
//			Retrieve references to external classes here and load from data from the TMX object
//
//		void update( sf::Uint32, const sf::Vector2f & )
//			Called continiously every frame regardless if the player is inside the object
//			NOTE: the coordinate inputted is relative to the object
//
//		void onEnter( sf::Uint32, const sf::Vector2f & )
//			Called once when the player enters the object
//			NOTE: the coordinate inputted is relative to the object
//
//		void whileInside( sf::Uint32, const sf::Vector2f & )
//			Called continuously while the player is inside the object
//			NOTE: the coordinate inputted is relative to the object
//
//		void onExit( sf::Uint32, const sf::Vector2f & )
//			Called once when the player exits the object
//			NOTE: the coordinate inputted is relative to the object
//
//		void onInteract( const sf::Vector2f & )
//			Called once when the player interacts with the object with the primary key (default: z)
//			Takes in the absolute coordinate that was interacted with
//			NOTE: the coordinate inputted is relative to the object
//
//		bool hasCollision( const sf::Vector2f & ) const [pure virtual]
//			Returns if the position at the inputted absolute coordinate has collision
//			NOTE: the coordinate inputted is relative to the object
//-------------------------------------------------------------------------

/***************************************************************************/

class Field : public Map::Object, res::TextureLoader<>
{
	static const int FIELD_SIZE = farm::field::WIDTH * farm::field::HEIGHT;
	std::vector< farm::field::Tile * > highlight;

	void clearHighlighted()
	{
		for ( farm::field::Tile * tile : highlight )
			tile->highlight = false;
		highlight.clear();
	}
	
	const sf::Vector2i convert( const sf::Vector2f & pos ) const
	{
		return sf::Vector2i( (unsigned) pos.x / 32U, (unsigned) pos.y / 32U );
	}

	void load( const Tmx::Object & )
	{
		// texture that contains (watered) tilled graphics
		loadTexture( "data/tilesets/crops.png" );
	}
	
	void onInteract( const sf::Vector2f & pos )
	{
		//DEBUG: till the field when used
		const sf::Vector2i fpos = convert( pos );
		farm::field::Tile & tile = farm::field::getTile( fpos.x, fpos.y );
		if ( tile.till > 0 )
			tile.water = true;
		else
			tile.till = 1U;
	}
	
	bool hasCollision( const sf::Vector2f & pos ) const
	{
		const sf::Vector2i fpos = convert( pos );
		const farm::field::Tile & tile = farm::field::getTile( fpos.x, fpos.y );
		return tile.object != nullptr && tile.object->hasCollision();
	}
	
	void draw( sf::RenderTarget & target, sf::RenderStates states ) const
	{
		using namespace bf::farm;
	
		states.transform *= getTransform();
		
		sf::Sprite sprite( getTexture() );
		const field::Tile * tiles = farm::field::getTiles();
		
		for ( int i = 0; i < FIELD_SIZE; i++ )
		{
			const field::Tile & tile = tiles[i];
			sprite.setPosition( i % farm::field::WIDTH * TILE_WIDTH, i / farm::field::WIDTH * TILE_HEIGHT );
			
			// draw till
			if ( tile.till > 0 )
			{
				sprite.setTextureRect( sf::IntRect( tile.water ? 32 : 0, 0, 32, 32 ) );
				target.draw( sprite, states );
			}

			// draw object
			if ( tile.object != nullptr )
				target.draw( *tile.object, states );
		}
	}
};

/***************************************************************************/

class Sign : public Map::Object, private res::TextureLoader<>
{
	std::string m_text;

	void onInteract( const sf::Vector2f& pos )
	{
		showText( m_text );
	}
	
	bool hasCollision( const sf::Vector2f& pos ) const 
	{ 
		return true; 
	}

	void load( const Tmx::Object& object )
	{
		const auto& list = object.GetProperties().GetList();

		//auto findTexture = list.find( "texture" );
		//auto findText = list.find( "text" );

		//TODO: error checking

		loadTexture( list.at( "texture" ) );
		m_text = list.at( "text" );
	}
	
	void draw( sf::RenderTarget& target, sf::RenderStates states ) const
	{
		states.transform *= getTransform();
		const sf::FloatRect& rect = getBounds();

		sf::Sprite sprite;
		sprite.setTexture( getTexture() );
		sprite.setTextureRect( sf::IntRect( 0, 0, (int) rect.width, (int) rect.height ) );

		target.draw( sprite, states );
	}
};

/***************************************************************************/

class UnknownObjectException : public Exception { public: UnknownObjectException( const Tmx::Object& object ) { *this << "unknown object type \"" << object.GetType() << "\""; } };

Map::Object * generateObject( const Tmx::Object & tmxObject )
{
	Map::Object * object = nullptr;

	try
	{
		std::string type = tmxObject.GetType();
		std::transform( type.begin(), type.end(), type.begin(), ::tolower );
		
		if ( type == "sign" )
			object = new Sign();
		else if ( type == "field" )
			object = new Field();
		else
			throw UnknownObjectException( tmxObject );
	
		object->setPosition( (float) tmxObject.GetX(), (float) tmxObject.GetY());

		object->m_name = tmxObject.GetName();
		object->m_bounds = sf::FloatRect( (float) tmxObject.GetX(), (float) tmxObject.GetY(), (float) tmxObject.GetWidth(), (float) tmxObject.GetHeight() );

		object->load( tmxObject );
		return object;
	}
	catch ( ... )
	{
		delete object;
		throw;
	}
}

/***************************************************************************/

} // namespace bf
