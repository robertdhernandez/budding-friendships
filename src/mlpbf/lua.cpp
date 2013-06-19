#include "mlpbf/console.h"
#include "mlpbf/console/command.h"
#include "mlpbf/global.h"
#include "mlpbf/lua.h"
#include "mlpbf/resource.h"

#include <cstring>
#include <iostream>

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

int texture_free( lua_State * l )
{
	lua::Texture * data = (lua::Texture *) luaL_checkudata( l, 1, TEXTURE_MT );
	data->~Texture();
	std::clog << "texture_free called" << std::endl;
	return 0;
}

static const struct luaL_Reg libtexture_mt [] =
{
	{ "__gc", texture_free },
	{ NULL, NULL },
};

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
	new (data) lua::Texture();
	
	luaL_getmetatable( l, TEXTURE_MT );
	lua_setmetatable( l, -2 );
	
	// set userdata variables from arguments
	data->texture  = texture;
	data->offset.x = x;
	data->offset.y = y;
	
	return 1;
}

int game_showText( lua_State * l )
{
	bf::showText( luaL_checkstring( l, 1 ), luaL_optstring( l, 2, "" ) );
	return 0;
}

static const struct luaL_Reg libgame[] = 
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
	else
		Console::singleton().setBufferColor( Console::INFO_COLOR );

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
	
	con::Command * cmd = new LuaCommand( l );
	Console::singleton().addCommand( cmd );
	Console::singleton() << con::setcinfo << "Lua: hooked console function " << cmd->name() << con::endl;

	return 0;
}

static const struct luaL_Reg libconsole[] = 
{
	{ "write", console_write },
	{ "execute", console_execute },
	{ "hook", console_hook },
	{ NULL, NULL },
};

/***************************************************************************/

lua_State * newState()
{
	// create lua state
	lua_State * l = luaL_newstate();
	luaL_openlibs( l );
	
	// texture metatable
	luaL_newmetatable( l, TEXTURE_MT );

	lua_pushvalue( l, -1 );
	lua_setfield( l, -2, "__index" );
	
	luaL_setfuncs( l, libtexture_mt, 0 );
	lua_pop( l, 1 );
	
	// register custom libraries
	luaL_newlib( l, libgame );
	lua_setglobal( l, "game" );

	luaL_newlib( l, libconsole );
	lua_setglobal( l, "console" );
	
	return l;
}

/***************************************************************************/

} // namespace lua

} // namespace bf
