#pragma once

#include "character.h"
#include "item/inventory.h"

namespace bf
{
	class Player : public Character
	{
	public:
		static Player& singleton();

		sf::Vector2f getUsePosition() const;

		item::Inventory& getInventory() { return m_inv; }
		const item::Inventory& getInventory() const { return m_inv; }

		sf::Uint8 getInventoryLevel() const { return m_invLevel; }	
		void setInventoryLevel( sf::Uint8 level );

	private:
		Player();

		sf::Uint8 m_invLevel;
		item::Inventory m_inv;
	};
}