#include "mlpbf/console.h"
#include "mlpbf/console/command.h"
#include "mlpbf/console/function.h"

#include "mlpbf/global.h"
#include "mlpbf/lua.h"
#include "mlpbf/player.h"
#include "mlpbf/time.h"
#include "mlpbf/exception.h"

#include <functional>
#include <sstream>

namespace bf
{
namespace con
{

/***************************************************************************/

class HELP : public con::Command
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
				throw Exception( "Unknown console command" );
			(*find)->help( c );
		}
	}
};

class CLEAR : public con::Command
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
};

class REPOSITION : public con::Command
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
};

class ANIMATE : public con::Command
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
};

class DEBUG_COLLISION : public con::Command
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
};

class TIME : public con::Command
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
};

class TIMESCALE : public con::Command
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
};

class SHOW_FPS : public con::Command
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
};

class MESSAGE : public con::Command
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
};

class LUA : public con::Command
{
	mutable bool executing;
	mutable lua_State * lua;

	const std::string name() const
	{
		return "lua";
	}
	
	unsigned minArgs() const
	{
		return 1;
	}
	
	void help( Console & c ) const
	{
		c << setcinfo << "Executes a lua script from the working directory" << con::endl;
		c << setcinfo << "lua \"filename\"" << con::endl;
	}
	
	void execute( Console & c, const std::vector< std::string > & args ) const
	{
		if ( executing )
			throw Exception( "Cannot execute lua console command recursively" );
			
		try
		{
			executing = true;
		
			if ( luaL_loadfile( lua, args[0].c_str() ) || lua_pcall( lua, 0, 0, 0 ) )
			{
				c << setcerr << lua_tostring( lua, -1 ) << con::endl;
				lua_pop( lua, 1 );
			}
		
			executing = false;
		}
		catch ( ... )
		{
			executing = false;
			throw;
		}
	}

public:
	LUA() : executing( false ), lua( lua::newState() ) {}
	~LUA() { lua_close( lua ); }
};

void defaultCommands( Console & console )
{
	console.addCommand( new HELP );
	console.addCommand( new CLEAR );
	console.addCommand( new REPOSITION );
	console.addCommand( new ANIMATE );
	console.addCommand( new DEBUG_COLLISION );
	console.addCommand( new TIME );
	console.addCommand( new SHOW_FPS );
	console.addCommand( new TIMESCALE );
	console.addCommand( new MESSAGE );
	console.addCommand( new LUA );
}

/***************************************************************************/

} // namespace con

} // namespace bf
