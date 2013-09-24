//#include "Game.h"

#include "mlpbf/global.h"
#include "mlpbf/direction.h"
#include "mlpbf/resource.h"

#include "mlpbf/database.h"
#include "mlpbf/farm.h"

#include "mlpbf/state/map.h"

#include "mlpbf/character.h"
#include "mlpbf/player.h"

#include "mlpbf/map.h"
#include "mlpbf/time/season.h"

#include "mlpbf/console.h"
#include "mlpbf/console/function.h"
#include "mlpbf/lua.h"

#include <SFML/System/Clock.hpp>
#include <SFML/Graphics.hpp>
#include <deque>

// NOTE:
// Microsoft Visual Studio C++ 2010 Redistributable required

bool bf::DEBUG_COLLISION = false;
bool bf::SHOW_FPS = true;

#ifdef MAIN_TRY_CATCH
#	ifdef _WIN32
#		include <Windows.h>
#	else
#		include <iostream>
#	endif
#endif

/***************************************************************************/

class FPS : public sf::Drawable, bf::res::FontLoader<>
{
public:
	void init()
	{
		m_frames = 0U;
		m_fps = 60U;
		loadFont( "data/fonts/console.ttf" );
	}

	void update()
	{
		if ( m_clock.getElapsedTime() >= sf::milliseconds( 1000 ) )
		{
			m_fps = m_frames;
			m_frames = 0;
			m_clock.restart();
		}
		m_frames++;
	}

	void draw( sf::RenderTarget& target, sf::RenderStates states ) const
	{
		if ( !bf::SHOW_FPS ) return;

		std::ostringstream fps;
		fps << m_fps;

		sf::Text text( fps.str(), getFont() );
		text.setColor( sf::Color::Yellow );

		target.draw( text );
	}

private:
	unsigned int m_frames, m_fps;
	sf::Clock m_clock;
} FPS;

/***************************************************************************/

class Fade : public sf::Drawable
{
	sf::Clock m_timer;
	sf::Time m_target;
	
	sf::Color m_color;
	
	int m_state;
	
	enum
	{
		NONE = 0,
		FADE_IN = 1,
		FADE_OUT = 2,
	};
	
public:
	Fade() : m_color( sf::Color( 0, 0, 0, 0 ) ) {}

	void fadeIn( sf::Time time )
	{
		m_target = time;
		m_state = FADE_IN;
		m_color = sf::Color( 0, 0, 0, 0 );
		m_timer.restart();
	}
	
	void fadeOut( sf::Time time )
	{
		m_target = time;
		m_state = FADE_OUT;
		m_color = sf::Color( 0, 0, 0, 255 );
		m_timer.restart();
	}
	
	void update()
	{
		if ( m_state != NONE )
		{
			unsigned ms1 = m_timer.getElapsedTime().asMilliseconds();
			unsigned ms2 = m_target.asMilliseconds();
			
			float p = std::min( (float) ms1 / ms2, 1.0f );
			if ( m_state == FADE_IN ) p = 1 - p;
			
			m_color.a = 255 * p;
			
			if ( ms1 >= ms2 )
				m_state = NONE;
		}
	}

	void draw( sf::RenderTarget & target, sf::RenderStates states ) const
	{
		sf::RectangleShape rect;
		rect.setSize( sf::Vector2f( bf::SCREEN_WIDTH, bf::SCREEN_HEIGHT ) );
		rect.setFillColor( m_color );
		
		target.draw( rect, states );
	}
};

/***************************************************************************/

void init()
{
	bf::res::init(); 	// resource managers
	bf::lua::init(); 	// lua
	bf::db::init(); 	// databases
	bf::farm::init(); 	// farm 
	
	FPS.init();
	
	// load console commands AFTER lua
	bf::con::defaultCommands( bf::Console::singleton() );
}

void cleanup()
{
	// clear console commands as some may require lua
	bf::Console::singleton().clearCommands();
	
	bf::farm::cleanup(); 	// farm
	bf::db::cleanup(); 		// databases
	bf::lua::cleanup(); 	// lua
	bf::res::cleanup(); 	// resource managers
}

/***************************************************************************/

static std::deque< const sf::Drawable * > g_Drawables;

void bf::showDrawable( const sf::Drawable * d )
{
	g_Drawables.push_back( d );
}

void bf::hideDrawable( const sf::Drawable * d )
{
	auto find = std::find( g_Drawables.begin(), g_Drawables.end(), d );
	if ( find != g_Drawables.end() )
		g_Drawables.erase( find );
}

/***************************************************************************/

static Fade ScreenTint;

void bf::fadeIn( sf::Time t )
{
	ScreenTint.fadeIn( t );
}

void bf::fadeOut( sf::Time t )
{
	ScreenTint.fadeOut( t );
}

/***************************************************************************/

int main( int argc, char* argv[] )
{
	using namespace bf;

#ifdef MAIN_TRY_CATCH
	try
	{
#endif
		init();

		sf::RenderWindow window( sf::VideoMode( SCREEN_WIDTH, SCREEN_HEIGHT ), "Budding Friendships", sf::Style::Close );
		window.setFramerateLimit( 60U );
		
		//TODO: make function to initialize all global variables
		Map::global( 0 );
		state::global( std::unique_ptr< state::Base >( new state::Map() ) );

		sf::Clock clock;
		Console& console = Console::singleton();

		Player::singleton().setMap( "path_a", sf::Vector2f( 448.0f , 448.0f ) );
		
		while ( window.isOpen() )
		{
			state::Base& state = state::global();

			sf::Event ev;
			while ( window.pollEvent( ev ) )
			{
				if ( ev.type == sf::Event::Closed )
					window.close();
				state.handleEvents( ev );
			}

			sf::Time time = clock.restart();
			state.update( time );
			if ( !Console::singleton().state() ) 
				lua::update( time.asMilliseconds() );
			FPS.update();
			ScreenTint.update();

			window.clear();

				window.draw( state );
				for ( const sf::Drawable * d : g_Drawables )
					window.draw( *d );
				window.draw( ScreenTint );
				window.draw( console );
				window.draw( FPS );

			window.display();
		}
		
		cleanup();

		return EXIT_SUCCESS;
#ifdef MAIN_TRY_CATCH
	}
	catch ( std::exception& err )
	{
#ifdef _WIN32
		MessageBox( NULL, err.what(), NULL, MB_OK | MB_ICONERROR );
#else
		std::cout << err.what() << std::endl;
#endif
		cleanup();
		return EXIT_FAILURE;
	}
#endif
}
