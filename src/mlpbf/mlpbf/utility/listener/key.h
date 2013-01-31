#pragma once

#include <SFML/Window/Event.hpp>

namespace bf
{
	namespace util
	{
		//-------------------------------------------------------------------------
		// Classes that inherit this interface can be interacted via keyboard input
		//-------------------------------------------------------------------------
		class KeyListener
		{
		public:
			KeyListener() : m_keyParent( nullptr ) {}
			virtual ~KeyListener() { unregisterKey(); }

			friend class KeyController;

			virtual void onKeyPressed( const sf::Event::KeyEvent& ) = 0;
			virtual void onKeyReleased( const sf::Event::KeyEvent& ) = 0;

		protected:
			void unregisterKey();

		private:
			KeyController* m_keyParent;
		};
	}
}