#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>

#include "../utility/timer.h"

namespace bf
{
	namespace ui
	{
		class Base : public sf::Drawable, public sf::Transformable
		{
		public:
			Base() : m_moving( false ) {}
			virtual ~Base() {}

			void update();

			void move( const sf::Vector2f& dest, const sf::Time& time );
			void move( const sf::Vector2f& start, const sf::Vector2f& dest, const sf::Time& time );

			bool isMoving() const { return m_moving; }

			void state( bool state ) { if ( !m_timer.finished() ) m_timer.setState( state ); }
			bool state() const { return !m_timer.finished() ? m_timer.getState() : false; }

		private:
			virtual void onUpdate() = 0;
		
		private:
			bool m_moving;
			sf::Vector2f m_start, m_end;
			util::Timer m_timer;
		};
	}
}