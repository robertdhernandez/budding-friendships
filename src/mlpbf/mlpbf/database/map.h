#pragma once

#include "base.h"
#include "../map.h"
#include <string>

namespace bf
{
	namespace db
	{
		class Map : public Database< bf::Map >
		{
		public:
			static db::Map& singleton();
			void initialize();

			// Non-const

			bf::Map& get( const std::string& i ) { return const_cast< bf::Map& >( static_cast< const db::Map& >( *this ).get( i ) ); }
			bf::Map& operator[]( const std::string& i ) { return get( i ); }

			bf::Map& get( unsigned i ) { return const_cast< bf::Map& >( static_cast< const db::Map& >( *this ).get( i ) ); }
			bf::Map& operator[]( unsigned i ) { return get( i ); }

			// Const

			using Database< bf::Map >::get;
			using Database< bf::Map >::operator[];

			const bf::Map& get( unsigned i ) const { return *m_ids.at( i ); }
			const bf::Map& operator[]( unsigned i ) const { return get( i ); }

		private:
			Map();

			const std::string getSourceFile() const { return "data/maps.xml"; }
			const std::string getDatabaseName() const { return "map database"; }
			const std::string getElementType() const { return "map"; }

			void load( const TiXmlElement& elem, bf::Map& map ) const;

		private:
			mutable std::vector< bf::Map* > m_ids;
		};
	}
}
