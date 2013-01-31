#pragma once

#include <bitset>
#include <string>

namespace bf
{
	namespace time
	{
		enum DayOfWeek
		{
			Sunday		= 0,
			Monday		= 1,
			Tuesday		= 2,
			Wednesday	= 3,
			Thursday	= 4,
			Friday		= 5,
			Saturday	= 6
		};

		typedef std::bitset< 7 > DaysOfWeek;

		DayOfWeek parseDayOfWeek( std::string );
		DaysOfWeek parseDaysOfWeek( std::string );
	}
}