#pragma once

#include <string>

namespace sf
{
	class RenderTarget;
}

namespace bf
{
	namespace time
	{
		//-------------------------------------------------------------------------
		// Can be instantiated by a string in format:
		//	XX:YY [AM/PM]
		//
		// If AM/PM is included, the string is read as a 12-hour clock (xE[1, 12])
		// Otherwise, it is read as a 24-hour clock (xE[0,23])
		// yE[0,59]
		//-------------------------------------------------------------------------
		class Hour
		{
		public:
			Hour() : m_time( 0 ) {}
			Hour( int hour, int minute ) { set( hour, minute ); }
			Hour( std::string time ) { set( time ); }

			void increment( int minutes );

			void set( int hour, int minute );
			void set( std::string );

			int get12Hour() const; // [1, 12]
			int get24Hour() const; // [0, 23]
			int getMinute() const; // [0, 59]

			unsigned short getRaw() const { return m_time; }

			bool isAM() const;
			bool isPM() const;

			const std::string to12HourString() const;
			const std::string to24HourString() const;
		
			bool operator>( const Hour& cmp ) const { return m_time > cmp.m_time; }
			bool operator<( const Hour& cmp ) const { return m_time < cmp.m_time; }
			bool operator==( const Hour& cmp ) const { return m_time == cmp.m_time; }

			bool operator>=( const Hour& cmp ) const { return ( *this > cmp || *this == cmp ); }
			bool operator<=( const Hour& cmp ) const { return ( *this < cmp || *this == cmp ); }
			bool operator!=( const Hour& cmp ) const { return !( *this == cmp ); }

			friend std::ostream& operator<<( std::ostream& s, const Hour& h ) { s << h.to12HourString(); return s; }

		private:
			unsigned short m_time;
		};

		extern const Hour MIDNIGHT;
		extern const Hour DAWN;
		extern const Hour NOON;
		extern const Hour DUSK;

		void drawHourTint( sf::RenderTarget&, const Hour& );
	}
}