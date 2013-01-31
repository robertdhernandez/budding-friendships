#pragma once

#include "base.h"
#include <vector>

namespace bf
{
	namespace item
	{
		class Inventory
		{
		public:
			Inventory( unsigned size = 0U );

			Item& addItem( const std::string& id );
			Item& addItem( ItemPtr item );
			Item& addItem( const std::string& id, unsigned index );
			Item& addItem( ItemPtr item, unsigned index );

			ItemPtr removeItem( unsigned index );

		public:
			bool hasLimit() const { return m_limit; }
			unsigned getSize() const { return m_items.size(); }

			void setLimit( unsigned size );

		public:
			ItemPtr& getSlot( unsigned index ) { return m_items.at( index ); }
			const ItemPtr& getSlot( unsigned index ) const { return m_items.at( index ); }

			ItemPtr& operator[]( unsigned index ) { return getSlot( index ); }
			const ItemPtr& operator[]( unsigned index ) const { return getSlot( index ); }

		private:
			bool m_limit;
			std::vector< ItemPtr > m_items;
		};
	}
}