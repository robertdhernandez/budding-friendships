#pragma once

#include <SFML/Window/Event.hpp>

namespace bf
{
	namespace util
	{
		class KeyListener;

		class KeyController
		{
		public:
			KeyController() : m_keyListener( nullptr ) {}
			virtual ~KeyController() { removeKeyListener(); }

			void setKeyListener( KeyListener& );
			void removeKeyListener();

		protected:
			void updateKeyListener( const sf::Event& );

		private:
			KeyListener* m_keyListener;
		};
	}
}