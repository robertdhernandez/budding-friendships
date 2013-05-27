#pragma once

#include "base.h"
#include "../resource.h"

namespace bf
{
	namespace data
	{
		class Item;
	}

	namespace item
	{
		class Generic : public item::Base, res::TextureLoader<>
		{
		public:
			Generic( const std::string& item, sf::Uint8 quality = 100U );
			Generic( const data::Item& data, sf::Uint8 quality = 100U );
			virtual ~Generic() {}

			ItemPtr clone() const;

			virtual void onUsePressed() {}
			virtual void onUseReleased() {}

			const std::string& getName() const;
			const std::string& getDesc() const;
			const std::string& getID() const;

			const sf::Texture& getIcon() const;

			unsigned getBuy() const;
			unsigned getSell() const;

			bool canRemove() const { return m_canRemove; }
			void remove() { m_canRemove = true; }

			sf::Uint8 getQuality() const { return m_quality; }

		private:
			bool m_canRemove;
			sf::Uint8 m_quality;
			const data::Item& m_data;
		};
	}
}
