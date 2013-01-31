#pragma once

#include "base.h"
#include "../utility/listener/key.h"
#include "../map/viewer.h"
#include "../ui/clock.h"

#include <array>

namespace bf
{
	enum Direction;

	namespace state
	{
		//-------------------------------------------------------------------------
		// [STATE]
		// This state is the most common state and will be played 90% of the time
		// It allows the player to move around the game world and interact with it
		//
		// Unlike the old PlayState, it does not personally manage the map, player, etc.
		// The state merely accesses and refers to the global variables
		//-------------------------------------------------------------------------
		class Map : public state::Base, public util::KeyListener
		{
		public:
			Map();

		private:
			void update( const sf::Time& );

			void onKeyPressed( const sf::Event::KeyEvent& );
			void onKeyReleased( const sf::Event::KeyEvent& );

			void draw( sf::RenderTarget&, sf::RenderStates ) const;

		private:
			map::MultiViewer m_viewer;
			ui::Clock m_clock;

			// Movement variables
			Direction m_dir;
			bool m_moving;
			enum { Default, Fast, Slow } m_speedModifier;

			// Boolean to update the player sprite if an input occured
			bool m_updateSprite;
		};
	}
}