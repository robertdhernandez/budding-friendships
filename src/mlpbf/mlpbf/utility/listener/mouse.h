#pragma once

#include <SFML/Window/Event.hpp>

namespace bf
{
	namespace util
	{
		//-------------------------------------------------------------------------
		// Classes that implement this interface can be interacted via mouse input
		//-------------------------------------------------------------------------
		class MouseListener
		{
		public:
			MouseListener() : m_mouseParent( nullptr ) {}
			virtual ~MouseListener() { unregisterMouse(); }

			friend class MouseController;

			virtual void onMouseButtonPressed( const sf::Event::MouseButtonEvent& ) = 0;
			virtual void onMouseButtonReleased( const sf::Event::MouseButtonEvent& ) = 0;
			virtual void onMouseEntered() = 0;
			virtual void onMouseLeft() = 0;
			virtual void onMouseMoved( const sf::Event::MouseMoveEvent& ) = 0;
			virtual void onMouseWheelMoved( const sf::Event::MouseWheelEvent& ) = 0;

		protected:
			void unregisterMouse();

		private:
			MouseController* m_mouseParent;
		};
	}
}