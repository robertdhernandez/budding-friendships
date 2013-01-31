#pragma once

#include <SFML/Window/Event.hpp>

namespace bf
{
	namespace util
	{
		class MouseListener;

		class MouseController
		{
		public:
			MouseController() : m_mouseListener( nullptr ) {}
			virtual ~MouseController() { removeMouseListener(); }

			void setMouseListener( MouseListener& );
			void removeMouseListener();

		protected:
			void updateMouseListener( const sf::Event& );

		private:
			MouseListener* m_mouseListener;
		};
	}
}