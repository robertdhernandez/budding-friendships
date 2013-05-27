#include "mlpbf/database/item.h"
#include "mlpbf/database/map.h"
#include "mlpbf/database/sprite.h"

#include "mlpbf/map.h"
#include "mlpbf/graphics/animation.h"
#include "mlpbf/graphics/spritesheet.h"

namespace bf
{

/***************************************************************************/

db::Item& db::Item::singleton()
{
	static db::Item db;
	return db;
}

void data::Item::load( const TiXmlElement& elem )
{
	m_id	= xml::attribute( elem, "id" );
	m_name	= xml::attribute( elem, "name" );
	m_desc	= xml::attribute( elem, "desc" );
	m_image	= xml::attribute( elem, "icon" );

	m_buy	= std::stoi( xml::attribute( elem, "buy" ) );
	m_sell	= std::stoi( xml::attribute( elem, "sell" ) );

	const TiXmlNode* it = nullptr;
	while ( it = elem.IterateChildren( "attribute", it ) )
	{
		const TiXmlElement& child = static_cast< const TiXmlElement& >( *it );
		m_attributes.insert( xml::attribute( child, "type" ) );
	}
}

/***************************************************************************/

db::Map& db::Map::singleton()
{
	static db::Map db;
	return db;
}

db::Map::Map()
{
	init();
}

void db::Map::initialize()
{
	for ( auto it = m_ids.begin(); it != m_ids.end(); ++it )
		(*it)->loadNeighbors();
}

void db::Map::load( const TiXmlElement& elem, bf::Map& map ) const
{
	map.load( m_ids.size(), xml::attribute( elem, "file" ) );
	m_ids.push_back( &map );
}

/***************************************************************************/

db::Sprite& db::Sprite::singleton()
{
	static db::Sprite db;
	return db;
}

void data::Sprite::load( const TiXmlElement& elem )
{
	m_file = xml::attribute( elem, "file" );
}

void data::Sprite::generate( gfx::Spritesheet& sheet ) const
{
	// Animations
	TiXmlDocument xml = xml::open( m_file );
	const TiXmlElement& root = *xml.RootElement();

	const TiXmlNode* it = nullptr;
	while ( it = root.IterateChildren( "animation", it ) )
	{
		std::unique_ptr< gfx::Animation > anim( new gfx::Animation( static_cast< const TiXmlElement& >( *it ) ) );
		std::string id = anim->getID();

		sheet.addAnimation( id, std::move( anim ) );
	}
}

/***************************************************************************/

} // namespace bf
