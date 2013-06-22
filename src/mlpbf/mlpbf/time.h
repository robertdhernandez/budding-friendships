#pragma once

#include "time/date.h"
#include "time/hour.h"
#include "time/clock.h"
#include <SFML/System/NonCopyable.hpp>

namespace bf
{
	class Time : public sf::NonCopyable
	{
	public:
		static Time& singleton();

		bool update();

		void setState( bool state ) { m_clock.setState( state ); }
		bool getState() const { return m_clock.getState(); }

		void setTimescale( float amt ) { m_clock.setTimescale( amt ); }

		inline time::Date & getDate() { return m_date; }
		inline time::Hour & getHour() { return m_hour; }
		
		inline const time::Date & getDate() const { return m_date; }
		inline const time::Hour & getHour() const { return m_hour; }

	private:
		Time();

		time::Date m_date;
		time::Hour m_hour;
		time::Clock m_clock;
	};
}
