#include "mlpbf/item/generic.h"
#include "mlpbf/item/inventory.h"

#include "mlpbf/exception.h"
#include "mlpbf/database/item.h"

#include <algorithm>

namespace bf
{
namespace item
{

/***************************************************************************/

class EmptyItemException : public Exception { public: EmptyItemException() { *this << "item cannot be null"; } };
class NoFreeSlotException : public Exception { public: NoFreeSlotException() { *this << "no free slot available"; } };
class IndexOutOfRangeException : public Exception { public: IndexOutOfRangeException( unsigned index, unsigned range ) { *this << "index " << index << " is out of range (" << range-1 << ")"; } };

/***************************************************************************/

Generic::Generic( const std::string& item, sf::Uint8 quality ) :
	m_canRemove( false ),
	m_quality( std::min( ( sf::Uint8 ) 100U, quality ) ),
	m_data( db::Item::singleton()[ item ] )
{
	loadTexture( m_data.getImage() );
}

Generic::Generic( const data::Item& item, sf::Uint8 quality ) :
	m_canRemove( false ),
	m_quality( std::min( ( sf::Uint8 ) 100U, quality ) ),
	m_data( item )
{
	loadTexture( m_data.getImage() );
}

ItemPtr Generic::clone() const
{
	return ItemPtr( new Generic( m_data, m_quality ) );
}

const std::string& Generic::getName() const
{
	return m_data.getName();
}

const std::string& Generic::getDesc() const
{
	return m_data.getDesc();
}

const std::string& Generic::getID() const
{
	return m_data.getID();
}

const sf::Texture& Generic::getIcon() const
{
	return getTexture();
}

unsigned Generic::getBuy() const
{
	return m_data.getBuy();
}

unsigned Generic::getSell() const
{
	return static_cast< unsigned >( m_data.getSell() * ( m_quality / 100.0f ) );
}

/***************************************************************************/

Inventory::Inventory( unsigned size ) :
	m_limit( false )
{
	setLimit( size );
}

Item& Inventory::addItem( const std::string& id )
{
	return addItem( ItemPtr( new Generic( id ) ) );
}

Item& Inventory::addItem( ItemPtr item )
{
	if ( !item )
		throw EmptyItemException();

	if ( m_limit )
	{
		// Find the first available index
		unsigned index;
		for ( index = 0; index < m_items.size(); index++ )
			if ( !m_items[ index ] )
				break;

		// Check it found a spot
		if ( index == m_items.size() )
			throw NoFreeSlotException();

		// Add the item
		m_items[ index ] = std::move( item );
		return *m_items[ index ];
	}
	else
	{
		m_items.push_back( item );
		return *m_items.back();
	}
}

Item& Inventory::addItem( const std::string& id, unsigned index )
{
	return addItem( ItemPtr( new Generic( id ) ), index );
}

Item& Inventory::addItem( ItemPtr item, unsigned index )
{
	if ( !item )
		throw EmptyItemException();

	// Ensure index is in bounds
	if ( index < 0 || m_items.size() <= index )
		throw IndexOutOfRangeException( index, m_items.size() );

	// Add the item
	m_items[ index ] = std::move( item );
	return *m_items[ index ];
}

ItemPtr Inventory::removeItem( unsigned index )
{
	if ( index < 0 || m_items.size() <= index )
		throw IndexOutOfRangeException( index, m_items.size() );

	ItemPtr remove( nullptr );
	std::swap( remove, m_items[ index ] );
	return remove;
}

void Inventory::setLimit( unsigned size )
{
	m_limit = ( size > 0U );
	if ( m_limit )
		m_items.resize( size, ItemPtr( nullptr ) );
}

/***************************************************************************/

} // namespace item

} // namespace bf