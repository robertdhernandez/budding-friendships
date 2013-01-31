#pragma once

#include "../utility/timer.h"

namespace bf
{
	namespace time
	{
		class Hour;

		class Clock
		{
		public:
			Clock( Hour& h );

			bool update();

			void setState( bool state ) { m_timer.setState( state ); }
			bool getState() const { return m_timer.getState(); }

			void setTimescale( float amt );

		private:
			Hour& m_hour;
			util::Timer m_timer;
		};
	}
}