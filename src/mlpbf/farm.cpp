#include "mlpbf/farm.h"
#include "mlpbf/exception.h"

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

const field::Tile & field::getTile( unsigned x, unsigned y )
{
	return g_Field.tiles[ convert( x, y ) ];
}

void field::plant( unsigned x, unsigned y, const Seed & seed )
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
