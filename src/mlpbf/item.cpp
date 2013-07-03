#include "mlpbf/database.h"
#include "mlpbf/exception.h"
#include "mlpbf/item.h"

namespace bf
{

/***************************************************************************/

class GenericItem : public Item
{
	const data::Item & m_data;
	unsigned char m_quantity, m_quality;

public:
	GenericItem( const data::Item & item, unsigned char quantity, unsigned char quality ) :
		m_data( item ),
		m_quantity( quantity ),
		m_quality( quality )
	{
	}
	
	Item * clone() const
	{
		return new GenericItem( m_data, m_quantity, m_quality );
	}
	
	void use()
	{
	}
	
	const std::string & getName() const
	{
		return m_data.name;
	}
	
	const std::string & getDesc() const
	{
		return m_data.desc;
	}
	
	const std::string & getID() const
	{
		return m_data.id;
	}
	
	unsigned getBuy() const
	{
		return m_data.buy;
	}
	
	unsigned getSell() const
	{
		return m_data.sell;
	}
	
	bool canRemove() const
	{
		return m_quantity == 0;
	}
};

/***************************************************************************/

Inventory::Inventory( unsigned size ) :
	m_hasLimit( size != 0U )
{
	if ( m_hasLimit )
		m_items.resize( size, nullptr );
}

Inventory::Inventory( const Inventory & copy ) :
	m_hasLimit( copy.m_hasLimit )
{
	m_items.resize( copy.m_items.size(), nullptr );
	for ( unsigned i = 0; i < copy.m_items.size(); i++ )
		if ( copy.m_items[i] != nullptr )
			m_items[i] = copy.m_items[i]->clone();
}

Inventory & Inventory::operator=( const Inventory & copy )
{
	for ( Item * item : m_items )
		delete item;
		
	m_hasLimit = copy.m_hasLimit;
	
	m_items.resize( copy.m_items.size(), nullptr );
	for ( unsigned i = 0; i < copy.m_items.size(); i++ )
		if ( copy.m_items[i] != nullptr )
			m_items[i] = copy.m_items[i]->clone();
	
	return *this;
}

Inventory::~Inventory()
{
	for ( Item * item : m_items )
		delete item;
}

void Inventory::addItem( Item * item )
{
	if ( m_hasLimit )
	{
		for ( unsigned i = 0; i < m_items.size(); i++ )
			if ( m_items[i] == nullptr )
			{
				m_items[i] = item;
				return;
			}
			
		delete item;
		throw Exception( "inventory does not contain a free slot" );
	}
	else
		m_items.push_back( item );
}

void Inventory::addItem( Item * item, unsigned index )
{
	try
	{
		if ( index >= m_items.size() )
			throw Exception( "inventory index out of bounds" );
			
		if ( m_items[index] != nullptr )
			throw Exception( "inventory index already occupied" );
			
		m_items[index] = item;
	}
	catch ( ... )
	{
		delete item;
		throw;
	}
}

Item * Inventory::getItem( unsigned index )
{
	return m_items.at( index );
}

const Item * Inventory::getItem( unsigned index ) const
{
	return m_items.at( index );
}

Item * Inventory::removeItem( unsigned index )
{
	Item * item = m_items.at( index );
	m_items[index] = nullptr;
	return item;
}

unsigned Inventory::getSize() const
{
	return m_items.size();
}

void Inventory::setSize( unsigned size )
{
	if ( ( m_hasLimit = ( size != 0U ) ) )
	{
		if ( size < m_items.size() )
			for ( unsigned i = size; i < m_items.size(); i++ )
				delete m_items[i];
		m_items.resize( size, nullptr );
	}
}

/***************************************************************************/

Item * generateItem( const std::string & id, unsigned char quantity, unsigned char quality )
{
	return new GenericItem( db::getItem( id ), quantity, quality );
}

/***************************************************************************/

} // namespace bf
