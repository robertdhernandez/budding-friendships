#include "mlpbf/console.h"
#include "mlpbf/global.h"
#include "mlpbf/lua.h"

namespace bf
{
namespace lua
{

/***************************************************************************/

int lua_showText( lua_State * l )
{
	bf::showText( luaL_checkstring( l, 1 ), luaL_optstring( l, 2, "" ) );
	return 0;
}

static const struct luaL_Reg testLib[] = 
{
	{ "showText", lua_showText },
	{ NULL, NULL },
};

/***************************************************************************/

// console.write( str [, col ] )
// prints a line to the console
int lua_consoleWrite( lua_State * l )
{
	if ( lua_gettop( l ) >= 2 && lua_isnumber( l, 2 ) )
		Console::singleton().setBufferColor( lua_tonumber( l, 2 ) );

	Console::singleton() << luaL_checkstring( l, 1 ) << con::endl;
	return 0;
}

static const struct luaL_Reg consoleLib[] = 
{
	{ "write", lua_consoleWrite },
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
	
	// register custom libraries
	registerLib( l, "test", testLib );
	registerLib( l, "console", consoleLib );
	
	return l;
}

/***************************************************************************/

} // namespace lua

} // namespace bf
