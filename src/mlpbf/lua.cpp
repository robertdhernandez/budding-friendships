#include "mlpbf/console.h"
#include "mlpbf/console/command.h"
#include "mlpbf/global.h"
#include "mlpbf/lua.h"
#include "mlpbf/resource.h"

#include <cstring>

namespace bf
{
namespace lua
{

/***************************************************************************/

struct Texture
{
	res::TexturePtr texture;
	sf::Vector2u offset;
};

static const char * TEXTURE_MT = "game.texture";

/***************************************************************************/

// game.loadTexture( str [, x, y ] )
int game_loadTexture( lua_State * l )
{
	// get arguments
	res::TexturePtr texture = res::loadTexture( luaL_checkstring( l, 1 ) );
	unsigned x = luaL_optinteger( l, 2, 0 );
	unsigned y = luaL_optinteger( l, 3, 0 );
	
	// create userdata and set metatable
	lua::Texture * data = (lua::Texture *) lua_newuserdata( l, sizeof( lua::Texture ) );
	luaL_getmetatable( l, TEXTURE_MT );
	lua_setmetatable( l, -2 );
	
	// set userdata variables from arguments
	memcpy( &data->texture, &texture, sizeof( res::TexturePtr ) );
	data->offset.x = x;
	data->offset.y = y;
	
	return 1;
}

int game_showText( lua_State * l )
{
	bf::showText( luaL_checkstring( l, 1 ), luaL_optstring( l, 2, "" ) );
	return 0;
}

static const struct luaL_Reg gameLib[] = 
{
	{ "loadTexture", game_loadTexture },
	{ "showText", game_showText },
	{ NULL, NULL },
};

/***************************************************************************/

// console.write( str [, col ] )
// prints a line to the console
int console_write( lua_State * l )
{
	if ( lua_gettop( l ) >= 2 && lua_isnumber( l, 2 ) )
		Console::singleton().setBufferColor( lua_tonumber( l, 2 ) );

	Console::singleton() << luaL_checkstring( l, 1 ) << con::endl;
	return 0;
}

// console.execute( str )
// executes a console commmand
// note: cannot execute lua console command
int console_execute( lua_State * l )
{
	Console::singleton().execute( luaL_checkstring( l, 1 ) );
	return 0;
}

int console_hook( lua_State * l )
{
	class LuaCommand : public con::Command
	{
		const std::string cmd;
		int ref;
		mutable lua_State * l;
		
		const std::string name() const 
		{ 
			return cmd; 
		}
		
		unsigned minArgs() const 
		{ 
			return 0;
		}
		
		void help( Console & ) const
		{
		}
		
		void execute( Console & c, const std::vector< std::string > & args ) const
		{
			lua_rawgeti( l, LUA_REGISTRYINDEX, ref );
			
			int numargs = 0;
			for ( const std::string & str : args )
			{
				lua_pushstring( l, str.c_str() );
				numargs++;
			}
			
			if ( lua_pcall( l, numargs, 0, 0 ) )
			{
				c << con::setcerr << lua_tostring( l, -1 ) << con::endl;
				lua_pop( l, 1 );
			}
		}
	
	public:
		LuaCommand( lua_State * L ) :
			cmd( lua_tostring( L, 1 ) ),
			ref( luaL_ref( L, LUA_REGISTRYINDEX ) ),
			l( L )
		{
		}
		
		~LuaCommand() { luaL_unref( l, LUA_REGISTRYINDEX, ref ); }		
	};
	
	luaL_checktype( l, 1, LUA_TSTRING );
	luaL_checktype( l, 2, LUA_TFUNCTION );
	
	Console::singleton().addCommand( new LuaCommand( l ) );

	return 0;
}

static const struct luaL_Reg consoleLib[] = 
{
	{ "write", console_write },
	{ "execute", console_execute },
	{ "hook", console_hook },
	{ NULL, NULL },
};

/***************************************************************************/

void registerLib( lua_State * l, const char * name, const struct luaL_Reg lib[] )
{
	// push a new table to the stack
	lua_newtable( l );
	
	for ( int i = 0; lib[i].name; i++ )
	{
		lua_pushcfunction( l, lib[i].func );
		lua_setfield( l, -2, lib[i].name );
	}
	
	// register the table in global index and pop the table
	lua_setglobal( l, name );
}

lua_State * newState()
{
	// create lua state
	lua_State * l = luaL_newstate();
	luaL_openlibs( l );
	
	// texture metatable
	luaL_newmetatable( l, TEXTURE_MT );
	lua_pop( l, 1 );
	
	// register custom libraries
	registerLib( l, "game", gameLib );
	registerLib( l, "console", consoleLib );
	
	return l;
}

/***************************************************************************/

} // namespace lua

} // namespace bf
