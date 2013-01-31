#pragma once

#include "base.h"
#include "../resource/loader/texture.h"

#include <set>

namespace bf
{
	namespace data
	{
		class Item
		{
		public:
			void load( const TiXmlElement& );

			const std::string& getID() const { return m_id; }
			const std::string& getName() const { return m_name; }
			const std::string& getDesc() const { return m_desc; }
			const std::string& getImage() const { return m_image; }

			int getBuy() const { return m_buy; }
			int getSell() const { return m_sell; }

			const std::set< std::string >& getAttributes() const { return m_attributes; }

		private:
			std::string m_id, m_name, m_desc, m_image;
			int m_buy, m_sell;
			std::set< std::string > m_attributes;
		};
	}

	namespace db
	{
		class Item : public Database< data::Item >
		{
		public:
			static Item& singleton();

		private:
			Item() { init(); }

			const std::string getSourceFile() const { return "data/items.xml"; }
			const std::string getDatabaseName() const { return "item database"; }
			const std::string getElementType() const { return "item"; }

			void load( const TiXmlElement& elem, data::Item& data ) const { data.load( elem ); }
		};
	}
}