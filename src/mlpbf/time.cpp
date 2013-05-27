#include "mlpbf/time.h"

#include "mlpbf/global.h"
#include "mlpbf/exception.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <iomanip>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace bf
{
namespace time
{

/***************************************************************************/

class DateOutOfRangeException : public Exception
{
public:
	DateOutOfRangeException( int d ) throw()
	{
		*this << "Day must be between 1 and 30: " << d;
	}
};

class EmptyDayOfWeekStringException : public Exception
{
public:
	EmptyDayOfWeekStringException( const std::string& str ) throw()
	{
		*this << "No day of week could be read from: " << str;
	}
};

class MultipleDaysOfWeekStringException : public Exception
{
public:
	MultipleDaysOfWeekStringException( const std::string& str ) throw()
	{
		*this << "Could not get a single day of a wekk because days of the week exist in: " << str;
	}
};

class YearOutOfRangeException : public Exception
{
public:
	YearOutOfRangeException( int d ) throw()
	{
		*this << "Year must be 1 or greater: " << d;
	}
};

class InvalidHourFormatException : public Exception
{
public:
	InvalidHourFormatException( const std::string& str ) throw()
	{
		*this << "Invalid hour format: " << str;
	}
};

class EmptySeasonStringException : public Exception
{
public:
	EmptySeasonStringException( const std::string& str ) throw()
	{
		*this << "No season could be read from: " << str;
	}
};

class MultipleSeasonStringException : public Exception
{
public:
	MultipleSeasonStringException( const std::string& str ) throw()
	{
		*this << "Could not get a single season because multiple seasons exist in: " << str;
	}
};

/***************************************************************************/
//	mlpbf/time/clock.h

static const sf::Time CLOCK_UPDATE_INTERVAL = sf::milliseconds( 500 );

Clock::Clock( Hour& h ) :
	m_hour( h )
{
	m_timer.setTarget( CLOCK_UPDATE_INTERVAL );
	m_timer.setState( true );
}

bool Clock::update()
{
	if ( m_timer )
	{
		m_hour.increment( 1 );
		m_timer.restart();
		return true;
	}
	return false;
}

void Clock::setTimescale( float amt )
{
	if ( amt > 0.0f )
	{
		m_timer.setTarget( CLOCK_UPDATE_INTERVAL * ( 1.0f / amt ) );
		m_timer.setState( true );
	}
	else if ( amt == 0.0f )
		m_timer.setState( false );
	else
		throw Exception( "Timescale must be greater than or equal to 0" );
}

/***************************************************************************/
//	mlpbf/time/date.h

void Date::set( Season s, int day, int year )
{
	if ( day < 0 || 29 < day )
		throw DateOutOfRangeException( day + 1 );

	year *= 120;
	int seasonOffset;

	switch ( s )
	{
	case Spring:	seasonOffset = 0; break;
	case Summer:	seasonOffset = 30; break;
	case Fall:		seasonOffset = 60; break;
	case Winter:	seasonOffset = 90; break;
	}

	m_day = seasonOffset + day + year;
}

void Date::set( std::string str )
{
	std::transform( str.begin(), str.end(), str.begin(), ::tolower );

	std::string::size_type pos = str.find( ' ' );
	std::string::size_type comma = str.find( "year", pos + 1 );
	bool hasYear = ( comma != std::string::npos );

	Season s = parseSeason( str.substr( 0, pos ) );
	int day = std::stoi( str.substr( pos + 1, ( hasYear ? comma - pos + 1 : std::string::npos ) ) ) - 1;

	int year = 0;
	if ( hasYear ) year = std::stoi( str.substr( comma + 5 ) ) - 1;
	if ( year < 0 ) throw YearOutOfRangeException( year );

	set( s, day, year );
}

Season Date::getSeason() const
{
	switch ( m_day / 30 % 4 )
	{
	case 0:	return Spring;
	case 1:	return Summer;
	case 2:	return Fall;
	case 3: return Winter;
	}

	throw Exception( "Date::getSeason reached an impossible location" );
}

DayOfWeek Date::getDayOfWeek() const
{
	switch ( m_day % 7 )
	{
	case 0:	return Sunday;
	case 1:	return Monday;
	case 2:	return Tuesday;
	case 3:	return Wednesday;
	case 4:	return Thursday;
	case 5:	return Friday;
	case 6:	return Saturday;
	}

	throw Exception( "Date::getDayOfWeek reached an impossible location" );
}

unsigned Date::getDay() const
{
	return m_day % 30 + 1;
}

unsigned Date::getYear() const
{
	return m_day / 120 + 1;
}

const std::string Date::toString() const
{
	std::ostringstream ss;

	switch ( getSeason() )
	{
	case Spring:	ss << "Spring ";	break;
	case Summer:	ss << "Summer ";	break;
	case Fall:		ss << "Fall ";		break;
	case Winter:	ss << "Winter ";	break;
	}

	ss << getDay() << " Year " << getYear();

	return ss.str();
}

int Date::compareAbs( const Date& cmp ) const
{
	return m_day - cmp.m_day;
}

int Date::compareRel( const Date& cmp ) const
{
	return ( m_day % 120 ) - ( cmp.m_day % 120 );
}

/***************************************************************************/
//	mlpbf/time/season.h

DayOfWeek parseDayOfWeek( std::string str )
{
	Seasons bit = parseSeasons( str );
	if ( bit.count() != 1 )
		throw MultipleSeasonStringException( str );
	
	if ( bit[ Sunday ] )	return Sunday;
	if ( bit[ Monday ] )	return Monday;
	if ( bit[ Tuesday ] )	return Tuesday;
	if ( bit[ Wednesday ] )	return Wednesday;
	if ( bit[ Thursday ] )	return Thursday;
	if ( bit[ Friday ] )	return Friday;
	if ( bit[ Saturday ] )	return Saturday;

	throw Exception( "parseDayOfWeek reached an impossible location" );
}

DaysOfWeek parseDaysOfWeek( std::string str )
{
	std::transform( str.begin(), str.end(), str.begin(), ::tolower );

	DaysOfWeek bit;

	if ( str.find( "sunday" ) != std::string::npos )
		bit[ Sunday ] = true;
	if ( str.find( "monday" ) != std::string::npos )
		bit[ Monday ] = true;
	if ( str.find( "tuesday" ) != std::string::npos )
		bit[ Tuesday ] = true;
	if ( str.find( "wednesday" ) != std::string::npos )
		bit[ Wednesday ] = true;
	if ( str.find( "thursday" ) != std::string::npos )
		bit[ Thursday ] = true;
	if ( str.find( "friday" ) != std::string::npos )
		bit[ Friday ] = true;
	if ( str.find( "saturday" ) != std::string::npos )
		bit[ Saturday ] = true;

	if ( bit.none() )
		throw EmptyDayOfWeekStringException( str );

	return bit;
}

/***************************************************************************/
//	mlpbf/time/hour.h (part 1)

const Hour MIDNIGHT( 0, 0 );
const Hour DAWN( 6, 0 );
const Hour NOON( 12, 0 );
const Hour DUSK( 18, 0 );

void Hour::increment( int minutes )
{
	m_time = ( m_time + minutes ) % ( 24 * 60 );
}

void Hour::set( int hour, int minute )
{
	if ( hour < 0 || 23 < hour )
		throw Exception( "Hour must be [0,23]" );
	if ( minute < 0 || 59 < minute )
		throw Exception( "Minute must be [0,59]" );

	m_time = hour * 60 + minute;
}

void Hour::set( std::string time )
{
	std::transform( time.begin(), time.end(), time.begin(), ::toupper );

	std::string::size_type pos = time.find( ':' );
	if ( pos == std::string::npos )
		throw InvalidHourFormatException( "missing \':\'" );

	std::string::size_type am = time.find( "AM" );
	std::string::size_type pm = time.find( "PM" );

	int hour, min;

	if ( am != std::string::npos && pm != std::string::npos )
		throw InvalidHourFormatException( "can only contain \"AM\", \"PM\" or neither" );
	else if ( am == std::string::npos && pm == std::string::npos ) // 24-hour clock
	{
		hour = std::stoi( time.substr( 0, pos ) );
		min = std::stoi( time.substr( pos + 1 ) );

		if ( hour < 0 || 23 < hour )
			throw InvalidHourFormatException( "hour must be between 0 and 23" );
	}
	else // 12-hour clock
	{
		std::string::size_type typePos = ( am != std::string::npos ) ? am : pm;
		
		hour = std::stoi( time.substr( 0, pos ) ) - 1;
		min = std::stoi( time.substr( pos + 1, typePos - pos + 1 ) );

		if ( hour < 0 || 11 < hour )
			throw InvalidHourFormatException( "hour must be between 1 and 12" );

		if ( pm != std::string::npos ) // Increment hour by 12 to adjust for PM
			hour = ( hour != 11 ) ? hour + 12 : 12; // Special case: 12:XX PM = 12
		else if ( am != std::string::npos && hour == 11 ) // Special case: 12:XX AM = 0
			hour = 0;
	}

	if ( min < 0 || 60 < min )
		throw InvalidHourFormatException( "minute must be between 0 and 59" );

	m_time = hour * 60 + min;
}

int Hour::get12Hour() const
{
	int hour = ( ( isPM() ) ? m_time - 12 * 60 : m_time ) / 60;
	return ( hour != 0 ) ? hour : 12;
}

int Hour::get24Hour() const
{
	return m_time / 60;
}

int Hour::getMinute() const
{
	return m_time % 60;
}

bool Hour::isAM() const
{
	return m_time < 12 * 60;
}

bool Hour::isPM() const
{
	return 12 * 60 <= m_time;
}

const std::string Hour::to12HourString() const
{
	std::ostringstream ss;
	ss << get12Hour() << ":" << std::setfill( '0' ) << std::setw( 2 ) << getMinute() << std::setw( 3 ) << ( isPM() ? " PM" : " AM" );
	return ss.str();
}

const std::string Hour::to24HourString() const
{
	std::ostringstream ss;
	ss << get24Hour() << ":" << std::setfill( '0' ) << std::setw( 2 ) << getMinute();
	return ss.str();
}

/***************************************************************************/
//	mlpbf/time/hour.h (part 2)

// Hour colors
const static sf::Color COLOR_SUNRISE	= sf::Color( 255, 102,   0,  64 );
const static sf::Color COLOR_DAY		= sf::Color( 255, 255, 255,   0 );
const static sf::Color COLOR_SUNSET	= sf::Color( 255, 102,   0,  64 );
const static sf::Color COLOR_NIGHT		= sf::Color( 32,   16,  64, 102 );

// Color time areas -- Previous color to Next color
typedef std::pair< Hour, Hour > Vector2H;
const static Vector2H TIME_NIGHT_TO_MORNING 	= Vector2H( Hour( 5,   0 ), Hour( 6,  30 ) );	// Night to Sunrise
const static Vector2H TIME_MORNING_TO_DAY	= Vector2H( Hour( 6,  30 ), Hour( 8,  0  ) );	// Sunrise to Day
const static Vector2H TIME_DAY			= Vector2H( Hour( 8,   0 ), Hour( 17, 0  ) );	// Day to Day
const static Vector2H TIME_DAY_TO_EVENING	= Vector2H( Hour( 17,  0 ), Hour( 18, 30 ) );	// Day to Sunset
const static Vector2H TIME_EVENING_TO_NIGHT	= Vector2H( Hour( 18, 30 ), Hour( 20, 0  ) );	// Sunset to Day
const static Vector2H TIME_NIGHT			= Vector2H( Hour( 20,  0 ), Hour( 5,  0  ) );	// Night to Night

static sf::Color transitionColor( const sf::Color& a, const sf::Color& b, float mult )
{
	using sf::Uint8;

	float inv = 1.f - mult;

	sf::Color ampA( static_cast< Uint8 >( a.r * inv ), 
				    static_cast< Uint8 >( a.g * inv ), 
				    static_cast< Uint8 >( a.b * inv ), 
				    static_cast< Uint8 >( a.a * inv ) );

	sf::Color ampB( static_cast< Uint8 >( b.r * mult ), 
				    static_cast< Uint8 >( b.g * mult ), 
				    static_cast< Uint8 >( b.b * mult ), 
				    static_cast< Uint8 >( b.a * mult ) );

	return sf::Color( ampA.r + ampB.r, ampA.g + ampB.g, ampA.b + ampB.b, ampA.a + ampB.a );
}

static sf::Color tintColor( const Hour& hour )
{
	const Vector2H*  period = nullptr;
	const sf::Color* color1 = nullptr;
	const sf::Color* color2 = nullptr;

	// Between 5 AM and 6 AM -- NIGHT_TO_MORNING
	if ( hour >= TIME_NIGHT_TO_MORNING.first && hour < TIME_NIGHT_TO_MORNING.second )
	{
		period = &TIME_NIGHT_TO_MORNING;
		color1 = &COLOR_NIGHT;
		color2 = &COLOR_SUNRISE;
	}

	// Betwen 6 AM and 8 AM -- MORNING_TO_DAY
	else if ( hour >= TIME_MORNING_TO_DAY.first && hour < TIME_MORNING_TO_DAY.second )
	{
		period = &TIME_MORNING_TO_DAY;
		color1 = &COLOR_SUNRISE;
		color2 = &COLOR_DAY;
	}

	// Between 8 AM and 5 PM -- DAY
	else if ( hour >= TIME_DAY.first && hour < TIME_DAY.second )
	{
		return COLOR_DAY;
	}

	// Between 5 PM and 6 PM -- DAY_TO_EVENING
	else if ( hour >= TIME_DAY_TO_EVENING.first && hour < TIME_DAY_TO_EVENING.second )
	{
		period = &TIME_DAY_TO_EVENING;
		color1 = &COLOR_DAY;
		color2 = &COLOR_SUNSET;
	}

	// Between 6 PM and 8 PM -- EVENING_TO_DAY
	else if ( hour >= TIME_EVENING_TO_NIGHT.first && hour < TIME_EVENING_TO_NIGHT.second )
	{
		period = &TIME_EVENING_TO_NIGHT;
		color1 = &COLOR_SUNSET;
		color2 = &COLOR_NIGHT;
	}

	// Between 8 PM and 5 AM -- NIGHT
	else
	{
		return COLOR_NIGHT;
	}

	assert( period );
	assert( color1 );
	assert( color2 );

	float a = static_cast< float >( ( hour.get24Hour() * 60 + hour.getMinute() ) - ( period->first.get24Hour() * 60 + period->first.getMinute() ) );
	float b = static_cast< float >( ( period->second.get24Hour() * 60 + period->second.getMinute() ) - ( period->first.get24Hour() * 60 + period->first.getMinute() ) );

	float mult = a / b;
	return transitionColor( *color1, *color2, mult );
}

void drawHourTint( sf::RenderTarget& target, const Hour& hour )
{
	float width = static_cast< float >( SCREEN_WIDTH );
	float height = static_cast< float >( SCREEN_HEIGHT );

	sf::Color color = tintColor( hour );

	sf::RectangleShape rect( sf::Vector2f( width, height ) );
	rect.setFillColor( color );

	target.draw( rect );
}

/***************************************************************************/
//	mlpbf/time/season.h

Season parseSeason( std::string str )
{
	Seasons bit = parseSeasons( str );
	if ( bit.count() != 1 )
		throw MultipleSeasonStringException( str );
	
	if ( bit[ Spring ] )	return Spring;
	if ( bit[ Summer ] )	return Summer;
	if ( bit[ Fall ] )		return Fall;
	if ( bit[ Winter ] )	return Winter;

	throw Exception( "parseSeason reached an impossible location" );
}

Seasons parseSeasons( std::string str )
{
	std::transform( str.begin(), str.end(), str.begin(), ::tolower );

	Seasons bit;

	if ( str.find( "spring" ) != std::string::npos )
		bit[ Spring ] = true;
	if ( str.find( "summer" ) != std::string::npos )
		bit[ Summer ] = true;
	if ( str.find( "fall" ) != std::string::npos )
		bit[ Fall ] = true;
	if ( str.find( "winter" ) != std::string::npos )
		bit[ Winter ] = true;

	if ( bit.none() )
		throw EmptySeasonStringException( str );

	return bit;
}

} // namespace time

/***************************************************************************/
//	mlpbf/time.h

Time& Time::singleton()
{
	static Time t;
	return t;
}

Time::Time() :
	m_date( 0 ),
	m_hour( time::DAWN ),
	m_clock( m_hour )
{
}

bool Time::update()
{
	bool updated = m_clock.update();
	if ( updated )
	{
		if ( m_hour == time::MIDNIGHT )
			m_date.increment( 1 );
	}
	return updated;
}

/***************************************************************************/

} // namespace bf
