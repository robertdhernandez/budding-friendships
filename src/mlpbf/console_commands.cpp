#include "mlpbf/console.h"
#include "mlpbf/console/command.h"
#include "mlpbf/console/function.h"

#include "mlpbf/global.h"
#include "mlpbf/player.h"
#include "mlpbf/time.h"

#include <functional>
#include <sstream>

namespace bf
{
namespace con
{

/***************************************************************************/

static class HELP : public con::Command
{
	const std::string name() const
	{
		return "help";
	}

	unsigned minArgs() const
	{
		return 0;
	}

	void help( Console& c ) const
	{
		c << setcinfo << "Displays the list of console commands" << endl;
	}

	void execute( Console& c, const std::vector< std::string >& args ) const
	{
		const auto& cmds = c.getCommands();

		if ( args.size() == 0 )
		{
			c << setcinfo << "Commands:" << endl;
			for ( auto it = cmds.begin(); it != cmds.end(); ++it )
				c << setcinfo << "\t" << (*it)->name() << endl;
		}
		else
		{
			auto find = std::find_if( cmds.begin(), cmds.end(), std::bind( &con::Command::operator==, std::placeholders::_1, args[ 0 ] ) );
			if ( find == cmds.end() )
				throw std::exception( "Unknown console command" );
			(*find)->help( c );
		}
	}
} HELP;

static class CLEAR : public con::Command
{
	const std::string name() const
	{
		return "clear";
	}

	unsigned minArgs() const 
	{ 
		return 0;
	}

	void help( Console& c ) const
	{
		c << setcinfo << "Clears the console" << endl;
	}

	void execute( Console& c, const std::vector< std::string >& args ) const
	{
		c.clearHistory();
	}
} CLEAR;

static class REPOSITION : public con::Command
{
	const std::string name() const
	{
		return "reposition";
	}

	unsigned minArgs() const
	{
		return 2;
	}

	void help( Console& c ) const
	{
		c << setcinfo << "Repositions the player" << endl;
		c << setcinfo << "reposition x y [map]" << endl;
	}

	void execute( Console& c, const std::vector< std::string >& args ) const
	{
		sf::Vector2f pos( std::stof( args[ 0 ] ), std::stof( args[ 1 ] ) );

		if ( args.size() >= 3 )
			Player::singleton().setMap( args[ 2 ], pos );
		else
			Player::singleton().setPosition( pos );
	}
} REPOSITION;

static class ANIMATE : public con::Command
{
	const std::string name() const
	{
		return "animate";
	}

	unsigned minArgs() const
	{
		return 1;
	}

	void help( Console& c ) const
	{
		c << setcinfo << "Plays a custom animation on the player" << endl;
		c << setcinfo << "animate anim [loop]" << endl;
		c << setcinfo << "anim: string\nloop: boolean, optional; default false" << endl;
	}

	void execute( Console& c, const std::vector< std::string >& args ) const
	{
		bool loop = false;
		if ( args.size() >= 2 )
			std::istringstream( args[ 1 ] ) >> std::boolalpha >> loop;
		Player::singleton().animate( args[ 0 ], loop );
	}
} ANIMATE;

static class DEBUG_COLLISION : public con::Command
{
	const std::string name() const
	{
		return "debug_collision";
	}

	unsigned minArgs() const
	{
		return 1;
	}

	void help( Console& c ) const
	{
		c << setcinfo << "Shows the collision box of characters" << endl;
		c << setcinfo << "debug_collision true/false" << endl;
	}

	void execute( Console& c, const std::vector< std::string >& args ) const
	{
		std::istringstream( args[ 0 ] ) >> std::boolalpha >> bf::DEBUG_COLLISION;
	}
} DEBUG_COLLISION;

static class TIME : public con::Command
{
	const std::string name() const
	{
		return "time";
	}

	unsigned minArgs() const
	{
		return 0;
	}

	void help( Console& c ) const
	{
	}

	void execute( Console& c, const std::vector< std::string >& args ) const
	{
		const Time& t = Time::singleton();
		if ( args.size() == 0 )
			c << t.getHour() << ", " << t.getDate() << con::endl;
	}
} TIME;

static class TIMESCALE : public con::Command
{
	const std::string name() const
	{
		return "timescale";
	}

	unsigned minArgs() const
	{
		return 1;
	}

	void help( Console& c ) const
	{
		c << setcinfo << "Changes the rate at which time moves" << con::endl;
	}

	void execute( Console&, const std::vector< std::string >& args ) const
	{
		Time::singleton().setTimescale( std::stof( args[ 0 ] ) );
	}
} TIMESCALE;

static class SHOW_FPS : public con::Command
{
	const std::string name() const
	{
		return "show_fps";
	}

	unsigned minArgs() const
	{
		return 1;
	}

	void help( Console& c ) const
	{
		c << setcinfo << "Toggles the display of the frames per second counter" << con::endl;
		c << setcinfo << "show_fps true/false" << con::endl;
	}

	void execute( Console& c, const std::vector< std::string >& args ) const
	{
		std::istringstream( args[ 0 ] ) >> std::boolalpha >> bf::SHOW_FPS;
	}
} SHOW_FPS;

static class MESSAGE : public con::Command
{
	const std::string name() const
	{
		return "message";
	}

	unsigned minArgs() const
	{
		return 1;
	}

	void help( Console& c ) const
	{
		c << setcinfo << "Displays a dialogue box" << con::endl;
		c << setcinfo << "message \"message\"" << con::endl;
		c << setcinfo << "message \"speaker\" \"message\"" << con::endl;
		c << setcinfo << "Note: use quotes (\"\") to input strings with spaces" << con::endl;
	}

	void execute( Console& c, const std::vector< std::string >& args ) const
	{
		if ( args.size() >= 2 )
			showText( args[ 1 ], args[ 0 ] );
		else if ( args.size() == 1 )
			showText( args[ 0 ] );
	}
} MESSAGE;

void defaultCommands( Console& console )
{
	console.addCommand( HELP );
	console.addCommand( CLEAR );
	console.addCommand( REPOSITION );
	console.addCommand( ANIMATE );
	console.addCommand( DEBUG_COLLISION );
	console.addCommand( TIME );
	console.addCommand( SHOW_FPS );
	console.addCommand( TIMESCALE );
	console.addCommand( MESSAGE );
}

/***************************************************************************/

} // namespace con

} // namespace bf