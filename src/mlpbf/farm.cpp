#include "mlpbf/exception.h"
#include "mlpbf/farm.h"
#include "mlpbf/global.h"
#include "mlpbf/resource.h"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <vector>

namespace bf
{
namespace farm
{

struct Field
{
	field::Tile * tiles;
	std::vector< field::Object * > objects;
} g_Field;

inline unsigned convert( unsigned x, unsigned y )
{
	return y * field::WIDTH + x;
}

inline bool areaClear( unsigned x, unsigned y, unsigned width, unsigned height )
{
	for ( int j = 0; j < height; j++ )
		for ( int i = 0; i < width; i++ )
			if ( g_Field.tiles[ convert( x + i, y + j ) ].object != nullptr )
				return false;
	return true;
}

/***************************************************************************/

class Crop : public field::Object
{
	const Seed & m_seed;
	const field::Tile & m_tile;

public:
	Crop( const Seed & seed, const field::Tile & tile ) :
		m_seed( seed ),
		m_tile( tile )
	{
	}
	
	void draw( sf::RenderTarget & target, sf::RenderStates states ) const
	{
	}
	
	bool hasCollision() const
	{
		return false;
	}
	
	unsigned getWidth() const
	{
		return 1;
	}
	
	unsigned getHeight() const
	{
		return 1;
	}
};

/***************************************************************************/

class Stone : public field::Object, sf::Transformable, res::TextureLoader<>
{
	unsigned m_size;
	
public:
	Stone( unsigned x, unsigned y, unsigned size ) : m_size( size )
	{
		if ( size < 1 || size > 3 )
			throw Exception( "stone must have size from 1 to 3" );
		
		if ( !areaClear( x, y, size, size ) )
			throw Exception( "area to place stone is not cleared" );
		
		switch ( size )
		{
			case 1: loadTexture( "data/farm/clutter/rock_s.png" ); break;
			case 2: loadTexture( "data/farm/clutter/rock_m.png" ); break;
			case 3: loadTexture( "data/farm/clutter/rock_l.png" ); break;
		}
		
		for ( int y = 0; y < size; y++ )
			for ( int x = 0; x < size; x++ )
				g_Field.tiles[ convert( x, y ) ].object = this;
				
		setPosition( x * TILE_WIDTH, y * TILE_HEIGHT );
	}
	
	void draw( sf::RenderTarget & target, sf::RenderStates states ) const
	{
		states.transform *= getTransform();
		target.draw( sf::Sprite( getTexture() ), states );
	}
	
	bool hasCollision() const
	{
		return true;
	}
	
	unsigned getWidth() const
	{
		return m_size;
	}
	
	unsigned getHeight() const
	{
		return m_size;
	}
};

/***************************************************************************/

void init()
{
	g_Field.tiles = new field::Tile[ field::WIDTH * field::HEIGHT ];
}

void cleanup()
{
	delete[] g_Field.tiles;
	for ( field::Object * obj : g_Field.objects )
		delete obj;
		
	g_Field.tiles = nullptr;
	g_Field.objects.clear();
}

/***************************************************************************/

field::Tile & field::getTile( unsigned x, unsigned y )
{
	return g_Field.tiles[ convert( x, y ) ];
}

field::Tile * field::getTiles()
{
	return g_Field.tiles;
}

const std::vector< field::Object * > & field::getObjects()
{
	return g_Field.objects;
}

/***************************************************************************/

void field::placeStone( unsigned x, unsigned y, unsigned size )
{
	g_Field.objects.push_back( new Stone( x, y, size ) );
}

void field::plantSeed( unsigned x, unsigned y, const Seed & seed )
{
	field::Tile & tile = g_Field.tiles[ convert( x, y ) ];
	if ( tile.object != nullptr )
		throw Exception( "tile already contains an object" );

	Crop * crop = new Crop( seed, tile );
	tile.object = crop;
	g_Field.objects.push_back( crop );
}

/***************************************************************************/

} // namespace farm

} // namespace bf
