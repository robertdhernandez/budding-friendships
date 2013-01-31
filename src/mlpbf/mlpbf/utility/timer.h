#pragma once

#include <algorithm>
#include <SFML/System/Clock.hpp>

namespace bf
{
	namespace util
	{
		//-------------------------------------------------------------------------
		// [UTILITY CLASS]
		//	The timer keeps track of time until it hits a certain point
		//-------------------------------------------------------------------------
		class Timer
		{
		public:
			Timer( bool state = false ) : m_active( state ), m_clock(), m_offset( sf::milliseconds( 0 ) ), m_target( sf::milliseconds( 0 ) ) {}

			bool getState() const { return m_active; }
			void setState( bool state ) { m_active = state; if ( !m_active ) m_offset = getElapsedTime(); else m_clock.restart(); }

			void setTarget( const sf::Time& target ) { m_target = target; restart(); }

			const sf::Time getElapsedTime() const { return ( m_active ) ? m_clock.getElapsedTime() + m_offset : m_offset; }
			const sf::Time getRemainingTime() const { return m_target - getElapsedTime(); }

			float getPercent() const { return std::min( 1.0f, (float) getElapsedTime().asMilliseconds() / m_target.asMilliseconds() ); }

			sf::Time restart() { sf::Time t = getElapsedTime(); m_clock.restart(); m_offset = sf::milliseconds( 0 ); return t; }

			bool finished() const { return getElapsedTime() >= m_target; }
			operator bool() const { return finished(); }

		private:
			bool m_active;
			sf::Clock m_clock;
			sf::Time m_offset, m_target;
		};
	}
}