#include "mlpbf/ui/base.h"
#include "mlpbf/ui/window.h"

#include "mlpbf/ui/clock.h"

#include "mlpbf/global.h"
#include "mlpbf/console.h"
#include "mlpbf/time/date.h"
#include "mlpbf/time/hour.h"

#include <cmath>
#include <sstream>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

namespace bf
{
namespace ui
{

inline float distance( const sf::Vector2f& a, const sf::Vector2f& b )
{
	return std::sqrt( std::pow( a.x - b.x, 2 ) + std::pow( a.y - b.y, 2 ) );
}

inline float angle( const sf::Vector2f& a, const sf::Vector2f& b )
{
	return atan2( b.y - a.y, b.x - a.x );
}

inline std::string to_string( int i )
{
	std::ostringstream ss;
	ss << i;
	return ss.str();
}

/***************************************************************************/
//	mlpbf/ui/base.h

void Base::move( const sf::Vector2f& dest, const sf::Time& time )
{
	m_moving = true;
	m_start = getPosition();
	m_end = dest;
	m_timer.setTarget( time );
	m_timer.setState( true );
}

void Base::move( const sf::Vector2f& start, const sf::Vector2f& dest, const sf::Time& time )
{
	setPosition( start );
	move( dest, time );
}

void Base::update()
{
	if ( m_moving )
	{
		float dist = distance( m_start, m_end ) * m_timer.getPercent();
		float theta = angle( m_start, m_end );
		setPosition( m_start.x + dist * cos( theta ), m_start.y + dist * sin( theta ) );
		m_moving = !m_timer.finished();
	}
	onUpdate();
}

/***************************************************************************/
//	mlpbf/ui/window.h

static std::unique_ptr< Window > GLOBAL_CONTAINER;

Window* Window::getGlobal()
{
	return GLOBAL_CONTAINER.get();
}

std::unique_ptr< Window > Window::setGlobal( Window* ct )
{
	auto old = std::move( GLOBAL_CONTAINER );
	GLOBAL_CONTAINER.reset( ct );
	return old;
}

Window::Window() :
	m_init( false ),
	m_state( Opening )
{
}

void Window::addChild( ui::Base* child )
{
	m_children.push_back( std::unique_ptr< ui::Base >( child ) );
}

ui::Base& Window::getChild( unsigned index )
{
	return *m_children.at( index );
}

void Window::close()
{
	if ( m_state == Opened )
	{
		m_init = false;
		m_state = Closing;
	}
}

bool Window::canRemove() const
{
	return m_state == Closed;
}

void Window::onUpdate()
{
	// Update the container
	switch ( m_state )
	{
	case Opening:
		if ( !m_init )
		{
			onOpen();
			m_init = true;
		}

		if ( opened() )	
			m_state = Opened;
	break;

	case Opened:
		// Update every children
		for ( auto it = m_children.begin(); it != m_children.end(); ++it )
			(*it)->update();
		onWindowUpdate();
	break;

	case Closing:
		if ( !m_init ) 
		{	
			onClose(); 
			m_init = true; 
		}

		if ( closed() )	
			m_state = Closed;
	break;
	}
}

void Window::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	states.transform *= getTransform();

	// Draw background
	target.draw( sf::Sprite( getTexture() ), states );

	// Draw the children
	for ( auto it = m_children.begin(); it != m_children.end(); ++it )
		target.draw( **it, states );
}

/***************************************************************************/
//	mlpbf/ui/clock.h

static const sf::Vector2f CLOCK_POS_WHEEL		= sf::Vector2f( 51.0f, 69.0f );
static const sf::Vector2f CLOCK_POS_SEASONS		= sf::Vector2f( 63.0f, 0.0f );
static const sf::Vector2f CLOCK_POS_DATE		= sf::Vector2f( 19.0f, 15.0f );

static const unsigned int CLOCK_DATE_SIZE		= 20U;
static const sf::Color CLOCK_DATE_COLOR			= sf::Color( 40, 88, 79 );

static const sf::Time CLOCK_HIDE_TIME			= sf::milliseconds( 250U );

enum
{
	CLOCK_TEXTURE_BG,
	CLOCK_TEXTURE_WHEEL,
	CLOCK_TEXTURE_SEASONS
};

static inline int getSeasonOffset( time::Season s, unsigned width )
{
	switch ( s )
	{
	default:
	case time::Spring:	return width * 0;
	case time::Summer:	return width * 1;
	case time::Fall:	return width * 2;
	case time::Winter:	return width * 3;
	}
}

Clock::Clock( const time::Date& date, const time::Hour& hour ) :
	m_visible( true ),
	m_date( date ),
	m_hour( hour )
{
	loadTexture( "data/ui/clock/bg.png", CLOCK_TEXTURE_BG );
	loadTexture( "data/ui/clock/wheel.png", CLOCK_TEXTURE_WHEEL );
	loadTexture( "data/ui/clock/seasons.png", CLOCK_TEXTURE_SEASONS );

	loadFont( "data/fonts/mvboli.ttf" );

	getTexture( CLOCK_TEXTURE_WHEEL ).setSmooth( true );

	setOrigin( 0.0f, getTexture( CLOCK_TEXTURE_BG ).getSize().y * 1.0f );
	setPosition( 0.0f, (float) SCREEN_HEIGHT );
}

void Clock::setVisible( bool visible )
{
	float size = getTexture( CLOCK_TEXTURE_BG ).getSize().x * -1.0f;

	float srcX  = visible ? size : 0.0f;
	float destX = visible ? 0.0f : size;
	float y = (float) SCREEN_HEIGHT;

	move( sf::Vector2f( srcX, y ), sf::Vector2f( destX, y ), CLOCK_HIDE_TIME );
	m_visible = visible;
}

bool Clock::isVisible() const
{
	return m_visible;
}

void Clock::onUpdate()
{
}

void Clock::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	states.transform *= getTransform();

	// Time Wheel
	sf::Sprite wheel( getTexture( CLOCK_TEXTURE_WHEEL ) );

	wheel.setOrigin( 48.0f, 48.0f );
	wheel.setPosition( CLOCK_POS_WHEEL );
	wheel.setRotation( -( ( m_hour.getRaw() / ( 24.0f * 60.0f ) * 360.f ) ) );

	// Date
	sf::Text date;

	date.setFont( getFont() );
	date.setColor( CLOCK_DATE_COLOR );
	date.setString( to_string( m_date.getDay() ) );
	date.setCharacterSize( CLOCK_DATE_SIZE );

	sf::FloatRect rect = date.getLocalBounds();
	date.setOrigin( rect.width / 2.0f, rect.height / 2.0f );
	date.setPosition( CLOCK_POS_DATE );

	// Season
	sf::Vector2u seasonSize = getTexture( CLOCK_TEXTURE_SEASONS ).getSize();
	seasonSize.x /= 4;

	sf::Sprite season( getTexture( CLOCK_TEXTURE_SEASONS ) );

	season.setPosition( CLOCK_POS_SEASONS );
	season.setTextureRect( sf::IntRect( getSeasonOffset( m_date.getSeason(), seasonSize.x ), 0, seasonSize.x, seasonSize.y ) );

	// Draw the sprites
	target.draw( wheel, states );
	target.draw( sf::Sprite( getTexture( CLOCK_TEXTURE_BG ) ), states );
	target.draw( date, states );
	target.draw( season, states );
}

/***************************************************************************/

} // namespace ui

} // namespace bf