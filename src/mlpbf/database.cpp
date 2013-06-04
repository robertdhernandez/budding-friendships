#include "mlpbf/database.h"
#include "mlpbf/exception.h"
#include "mlpbf/map.h"
#include "mlpbf/graphics/animation.h"
#include "mlpbf/graphics/spritesheet.h"
#include "mlpbf/time/season.h"
#include "mlpbf/xml.h"

#include <memory>
#include <string>
#include <tinyxml.h>
#include <unordered_map>

#include <SFML/System/NonCopyable.hpp>

namespace bf
{

/***************************************************************************/

template< typename T >
class Database : sf::NonCopyable
{
public:
	virtual ~Database() {}

	virtual void init()
	{
		TiXmlDocument xml = xml::open( getSourceFile() );
		const TiXmlElement& root = *xml.RootElement();
		
		const std::string element = getElementType();

		const TiXmlNode * it = nullptr;
		while ( it = root.IterateChildren( element.c_str(), it ) )
		{
			const TiXmlElement& elem = static_cast< const TiXmlElement& >( *it );

			std::string id = xml::attribute( elem, "id" );

			std::unique_ptr< T > entry( new T() );
			load( elem, *entry );

			m_data.insert( std::make_pair( id, std::move( entry ) ) );
		}
	}

	const T & get( const std::string & id ) const
	{
		auto find = m_data.find( id );
		if ( find == m_data.end() )
			throw InvalidElementException( id );
		return *find->second.get();
	}
	
	class InvalidElementException : public Exception { public: InvalidElementException( const std::string & id ) throw() { *this << "Cannot find " << id; } };

private:
	virtual const std::string getSourceFile() const = 0;
	virtual const std::string getDatabaseName() const = 0;
	virtual const std::string getElementType() const = 0;

	virtual void load( const TiXmlElement &, T & ) const = 0;

private:
	std::unordered_map< std::string, std::unique_ptr< T > > m_data;
};

/***************************************************************************/

class CropDatabase : public Database< data::Crop >
{
	const std::string getSourceFile() const
	{
		return "data/crops.xml";
	}
	
	const std::string getDatabaseName() const
	{
		return "crop database";
	}
	
	const std::string getElementType() const
	{
		return "crop";
	}
	
	void load( const TiXmlElement & elem, data::Crop & data ) const
	{
		/*
			string id;
			string seedID;
			string cropID; 
			string image;
			Seasons seasons;
			unsigned regrowth;
			std::vector< unsigned > growth;
		*/
		
		data.id 		= xml::attribute( elem, "id" );
		data.seed 	= &db::getItem( xml::attribute( elem, "seedID" ) );
		data.cropID 	= &db::getItem( xml::attribute( elem, "cropID" ) );
		data.image 	= xml::attribute( elem, "image" );
		data.seasons	= time::parseSeasons( xml::attribute( elem, "season" ) );
		data.regrowth	= std::stoi( xml::attribute( elem, "regrowth" ) );
		
		const TiXmlNode * it = nullptr;
		while ( it = elem.IterateChildren( "stage", it ) )
			data.growth.push_back( std::stoi( xml::attribute( static_cast< const TiXmlElement & >( *it ), "length" ) ) );
			
		if ( data.regrowth != 0 && data.regrowth >= data.growth.size() )
			throw Exception( "regrowth index is invalid" );
	}
} * g_dbCrop = nullptr;

/***************************************************************************/

class ItemDatabase : public Database< data::Item >
{
	const std::string getSourceFile() const 
	{
		return "data/items.xml"; 
	}
	
	const std::string getDatabaseName() const 
	{ 
		return "item database"; 
	}
	
	const std::string getElementType() const 
	{ 
		return "item"; 
	}

	void load( const TiXmlElement & elem, data::Item & data ) const 
	{ 
		data.id		= xml::attribute( elem, "id" );
		data.name		= xml::attribute( elem, "name" );
		data.desc		= xml::attribute( elem, "desc" );
		data.image	= xml::attribute( elem, "icon" );

		data.buy		= std::stoi( xml::attribute( elem, "buy" ) );
		data.sell		= std::stoi( xml::attribute( elem, "sell" ) );

		const TiXmlNode* it = nullptr;
		while ( it = elem.IterateChildren( "attribute", it ) )
		{
			const TiXmlElement& child = static_cast< const TiXmlElement& >( *it );
			data.attributes.insert( xml::attribute( child, "type" ) );
		}
	}
} * g_dbItem = nullptr;

/***************************************************************************/

class MapDatabase : public Database< bf::Map >
{
	mutable std::vector< bf::Map * > m_ids;

	const std::string getSourceFile() const 
	{ 
		return "data/maps.xml"; 
	}
	
	const std::string getDatabaseName() const 
	{ 
		return "map database"; 
	}
	
	const std::string getElementType() const 
	{ 
		return "map";
	}
	
	void load( const TiXmlElement & elem, bf::Map & map ) const
	{
		map.load( m_ids.size(), xml::attribute( elem, "file" ) );
		m_ids.push_back( &map );
	}

public:
	void init()
	{
		Database< bf::Map >::init();
		
		for ( bf::Map * map : m_ids )
			map->loadNeighbors();
	}

	bf::Map & getFromID( unsigned i )
	{ 
		return *m_ids[ i ]; 
	}
} * g_dbMap = nullptr;

/***************************************************************************/

class SpriteDatabase : public Database< std::string >
{
	const std::string getSourceFile() const 
	{ 
		return "data/sprites.xml";
	}
	
	const std::string getDatabaseName() const 
	{ 
		return "sprite database";
	}
	
	const std::string getElementType() const 
	{ 
		return "sprite";
	}

	void load( const TiXmlElement& elem, std::string & file ) const 
	{ 
		file = xml::attribute( elem, "file" );
	}
	
public:
	void generate( const std::string & id, gfx::Spritesheet * sheet )
	{
		// Animations
		TiXmlDocument xml = xml::open( get( id ) );
		const TiXmlElement& root = *xml.RootElement();

		const TiXmlNode * it = nullptr;
		while ( it = root.IterateChildren( "animation", it ) )
		{
			std::unique_ptr< gfx::Animation > anim( new gfx::Animation( static_cast< const TiXmlElement& >( *it ) ) );
			std::string id = anim->getID();

			sheet->addAnimation( id, std::move( anim ) );
		}
	}
} * g_dbSprite = nullptr;

/***************************************************************************/

void db::init()
{
	g_dbItem = new ItemDatabase();
	g_dbItem->init();
	
	g_dbMap = new MapDatabase();
	g_dbMap->init();
	
	g_dbSprite = new SpriteDatabase();
	g_dbSprite->init();
}

void db::cleanup()
{
	delete g_dbSprite;
	delete g_dbMap;
	delete g_dbItem;
	
	g_dbItem = nullptr;
	g_dbMap = nullptr;
	g_dbSprite = nullptr;
}

/***************************************************************************/

const data::Item & db::getItem( const std::string & id )
{
	return g_dbItem->get( id );
}

void db::genSprite( const std::string & id, gfx::Spritesheet * sheet )
{
	g_dbSprite->generate( id, sheet );
}

bf::Map & db::getMap( unsigned id )
{
	return g_dbMap->getFromID( id );
}

bf::Map & db::getMap( const std::string & id )
{
	return const_cast< bf::Map & >( g_dbMap->get( id ) );
}

/***************************************************************************/

} // namespace bf
