#include "mlpbf/console.h"
#include "mlpbf/console/command.h"
#include "mlpbf/console/function.h"
#include "mlpbf/database.h"
#include "mlpbf/global.h"
#include "mlpbf/lua.h"
#include "mlpbf/map.h"
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

class Help : public con::Command
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

class Clear : public con::Command
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

class Reposition : public con::Command
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

class Animate : public con::Command
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

class DebugCollision : public con::Command
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

class GetTime : public con::Command
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
		const bf::Time & t = bf::Time::singleton();
		if ( args.size() == 0 )
			c << t.getHour() << ", " << t.getDate() << con::endl;
	}
};

class Timescale : public con::Command
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

class ShowFPS : public con::Command
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

class Message : public con::Command
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

class Lua : public con::Command
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
	Lua() : executing( false ), lua( lua::state() ) {}
};

class ReloadMapObject : public con::Command
{
	const std::string name() const
	{
		return "reload_map_object";
	}
	
	unsigned minArgs() const
	{
		return 2;
	}
	
	void help( Console & c ) const
	{
		c << setcinfo << "Reloads an already loaded map object" << con::endl;
		c << setcinfo << "Useful for reloading a map object loaded from a Lua script" << con::endl;
		c << setcinfo << "reload_map_object map obj" << con::endl;
	}
	
	void execute( Console & c, const std::vector< std::string > & args ) const
	{
		db::getMap( args[0] ).reloadObject( args[1] );
	}
};

class Save : public con::Command
{
	const std::string name() const
	{
		return "save";
	}
	
	unsigned minArgs() const
	{
		return 1;
	}
	
	void help( Console & c ) const
	{
		c << setcinfo << "Saves data to a save file" << con::endl;
		c << setcinfo << "save filename" << con::endl;
	}
	
	void execute( Console & c, const std::vector< std::string > & args ) const
	{
		FILE * fp = fopen( args[0].c_str(), "wb" );
		lua::save( fp );
		fclose( fp );
		
		c << setcinfo << "Saved to " << args[0] << con::endl;
	}
};

class Load : public con::Command
{
	const std::string name() const
	{
		return "load";
	}
	
	unsigned minArgs() const
	{
		return 1;
	}
	
	void help( Console & c ) const
	{
		c << setcinfo << "Loads save data from a file" << con::endl;
		c << setcinfo << "load filename" << con::endl;
	}
	
	void execute( Console & c, const std::vector< std::string > & args ) const
	{
		FILE * fp = fopen( args[0].c_str(), "rb" );
		if ( fp == NULL )
			throw Exception( "File not found" );
		lua::load( fp );
		fclose( fp );
	}
};

void defaultCommands( Console & console )
{
	console.addCommand( new Help );
	console.addCommand( new Clear );
	console.addCommand( new Reposition );
	console.addCommand( new Animate );
	console.addCommand( new DebugCollision );
	console.addCommand( new GetTime );
	console.addCommand( new ShowFPS );
	console.addCommand( new Timescale );
	console.addCommand( new Message );
	console.addCommand( new Lua );
	console.addCommand( new ReloadMapObject );
	console.addCommand( new Save );
	console.addCommand( new Load );
}

/***************************************************************************/

} // namespace con

} // namespace bf
