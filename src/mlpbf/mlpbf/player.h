#pragma once

#include "character.h"
#include "item.h"

namespace bf
{
	class Player : public Character
	{
	public:
		static Player& singleton();

		sf::Vector2f getUsePosition() const;

		Inventory& getInventory() { return m_inv; }
		const Inventory& getInventory() const { return m_inv; }

		sf::Uint8 getInventoryLevel() const { return m_invLevel; }	
		void setInventoryLevel( sf::Uint8 level );

	private:
		Player();

		sf::Uint8 m_invLevel;
		Inventory m_inv;
	};
}
