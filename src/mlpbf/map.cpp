#include "mlpbf/map.h"
#include "mlpbf/map/object.h"
#include "mlpbf/map/viewer.h"

#include "mlpbf/global.h"
#include "mlpbf/console.h"
#include "mlpbf/character.h"
#include "mlpbf/direction.h"
#include "mlpbf/database/map.h"
#include "mlpbf/resource/manager/texture.h"
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
		throw std::out_of_range( "x-position is out of map bounds" );
	if ( pos.y < 0 || height <= pos.y )
		throw std::out_of_range( "y-position is out of map bounds" );
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

		m.first = &db::Map::singleton()[ _map ];
		if ( pos != std::string::npos )
			std::istringstream( find->second.substr( pos + 1 ) ) >> m.second;
	}
}

/***************************************************************************/

static Map* GLOBAL_MAP = nullptr;

Map& Map::global()
{
	if ( !GLOBAL_MAP )
		throw std::logic_error( "No map loaded!" );
	return *GLOBAL_MAP;
}

Map& Map::global( unsigned id )
{
	return *( GLOBAL_MAP = &db::Map::singleton()[ id ] );
}

Map& Map::global( const std::string& map )
{
	return *( GLOBAL_MAP = &db::Map::singleton()[ map ] );
}

/***************************************************************************/

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
		const map::Object& obj = **it;
		if ( obj.getBounds().contains( pos ) && obj.hasCollision( obj.getPosition() - pos ) )
			return true;
	}
	return false;
}

void Map::load( unsigned id, const std::string& map )
{
	m_mapID = id;
	m_map.ParseFile( map );

	if ( m_map.HasError() )
		throw std::exception( m_map.GetErrorText().c_str() );

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

		std::shared_ptr< sf::Texture > texture = res::TextureManager::singleton().load( file );
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
				m_objects.push_back( map::Object::create( object ) );
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
	std::vector< map::Object* > remove;

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
		(*it)->onInside( frameTime, pos - (*it)->getPosition() );

	// Check if the player has entered any new objects
	for ( auto it = m_objects.begin(); it != m_objects.end(); ++it )
	{
		const sf::FloatRect& rect = (*it)->getBounds();
		if ( rect.contains( pos ) && std::find( m_activeObjects.begin(), m_activeObjects.end(), it->get() ) != m_activeObjects.end() )
		{
			(*it)->onEnter( frameTime, pos - (*it)->getPosition() );
			m_activeObjects.push_back( it->get() );
		}
	}
}

bool Map::interact( const sf::Vector2f& pos )
{
	for ( auto it = m_objects.begin(); it != m_objects.end(); ++it )
		if ( (*it)->getBounds().contains( pos ) )
		{
			(*it)->onInteract( pos - (*it)->getPosition() );
			return true;
		}
	return false;
}

/***************************************************************************/

map::Viewer::Viewer( const Map& map ) :
	m_map( &map ),
	m_area( 0.0f, 0.0f, (float) SCREEN_WIDTH, (float) SCREEN_HEIGHT )
{
}

void map::Viewer::center( const sf::Vector2f& pos )
{
	sf::Vector2f p = round( pos );
	m_area.left = p.x - m_area.width  / 2.0f;
	m_area.top  = p.y - m_area.height / 2.0f;
}

const sf::Vector2f map::Viewer::center() const
{
	return sf::Vector2f( m_area.left - m_area.width / 2.0f, m_area.top - m_area.height / 2.0f );
}

void map::Viewer::dimension( const sf::Vector2f& dim )
{
	m_area.width = dim.x;
	m_area.height = dim.y;
}

void map::Viewer::draw( sf::RenderTarget& target, sf::RenderStates states ) const
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
			map::Object& object = const_cast< map::Object& >( **it );
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

void map::MultiViewer::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	const Map& m = map();
	const sf::FloatRect& area = getViewArea();
	sf::Vector2f center = this->center(), offset;

	Viewer child( *this );

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
	Viewer::draw( target, states );	
}

/***************************************************************************/

} // namespace bf