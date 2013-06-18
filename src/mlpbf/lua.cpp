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

lua_State * newState()
{
	// create lua state
	lua_State * l = luaL_newstate();
	luaL_openlibs( l );
	
	// register custom libraries
	lua_pushcfunction( l, lua_showText );
	lua_setglobal( l, "showText" );
	
	return l;
}

/***************************************************************************/

} // namespace lua

} // namespace bf
