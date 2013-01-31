#pragma once

#include <string>
#include "day_of_week.h"
#include "season.h"

namespace bf
{
	namespace time
	{
		//-------------------------------------------------------------------------
		// Can be instantiated by a string in format:
		//	Season Date ["Year" Year]
		//
		// Examples:
		//	Spring 29
		//	Fall 2
		//	Winter 10 Year 2
		//	Summer 15 Year 5
		//-------------------------------------------------------------------------
		class Date
		{
		public:
			Date( unsigned day = 0U ) { set( day ); }
			Date( Season s, int day ) { set( s, day ); }
			Date( std::string str ) { set( str ); }

			void increment( unsigned days ) { m_day += days; }

			void set( unsigned day ) { m_day = day; }
			void set( Season season, int day, int year = 0 );
			void set( std::string str );

			Season getSeason() const;
			DayOfWeek getDayOfWeek() const;

			unsigned getDay() const; // [1, 30]
			unsigned getYear() const;

			const std::string toString() const;

			int compareAbs( const Date& ) const; // Compares a date w/ regard to year
			int compareRel( const Date& ) const; // Compares a date w/o regard to year

			friend std::ostream& operator<<( std::ostream& s, const Date& d ) { s << d.toString(); return s; }

		private:
			unsigned m_day;
		};
	}
}