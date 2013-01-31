#pragma once

#include <bitset>
#include <string>

namespace bf
{
	namespace time
	{
		enum Season
		{
			Spring	= 0,
			Summer	= 1,
			Fall	= 2,
			Winter	= 3
		};

		typedef std::bitset< 4 > Seasons;

		Season parseSeason( std::string );
		Seasons parseSeasons( std::string );
	}
}