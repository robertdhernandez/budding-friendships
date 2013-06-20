#include "mlpbf/console.h"
#include "mlpbf/console/command.h"
#include "mlpbf/exception.h"
#include "mlpbf/global.h"
#include "mlpbf/lua.h"
#include "mlpbf/resource.h"
#include "mlpbf/time.h"

#include <cstring>
#include <iostream>
#include <SFML/Graphics/Sprite.hpp>
#include <unordered_map>

namespace bf
{
namespace lua
{

void addLuaRef( const std::string & str, int ref );
void removeLuaRef( const std::string & str );

/***************************************************************************/

struct Image
{
	res::TexturePtr texture;
	sf::Sprite sprite;
	bool display;
};

static const char * IMAGE_MT = "game.image";

static int image_display( lua_State * l )
{
	lua::Image * image = (lua::Image *) luaL_checkudata( l, 1, IMAGE_MT );
	luaL_checktype( l, 2, LUA_TBOOLEAN );
	
	if ( image->display = lua_toboolean( l, 2 ) )
		showDrawable( &image->sprite );
	else
		hideDrawable( &image->sprite );
	
	return 0;
}

static int image_load( lua_State * l )
{
	lua::Image * image = (lua::Image *) luaL_checkudata( l, 1, IMAGE_MT );
	
	image->texture = res::loadTexture( luaL_checkstring( l, 2 ) );
	image->sprite.setTexture( *image->texture );
	
	return 0;
}

static int image_move( lua_State * l )
{
	lua::Image * image = (lua::Image *) luaL_checkudata( l, 1, IMAGE_MT );
	float x = luaL_checknumber( l, 2 );
	float y = luaL_checknumber( l, 3 );
	
	image->sprite.move( x, y );
	
	return 0;
}

static int image_origin( lua_State * l )
{
	lua::Image * image = (lua::Image *) luaL_checkudata( l, 1, IMAGE_MT );
	
	if ( lua_gettop( l ) == 3 )
	{
		float x = luaL_checknumber( l, 2 );
		float y = luaL_checknumber( l, 3 );
		
		image->sprite.setOrigin( x, y );
	}
	
	const sf::Vector2f & origin = image->sprite.getOrigin();
	lua_pushnumber( l, origin.x );
	lua_pushnumber( l, origin.y );
	
	return 2;
}

static int image_position( lua_State * l )
{
	lua::Image * image = (lua::Image *) luaL_checkudata( l, 1, IMAGE_MT );
	
	if ( lua_gettop( l ) == 3 )
	{
		float x = luaL_checknumber( l, 2 );
		float y = luaL_checknumber( l, 3 );
		
		image->sprite.setPosition( x, y );
	}

	const sf::Vector2f & pos = image->sprite.getPosition();
	lua_pushnumber( l, pos.x );
	lua_pushnumber( l, pos.y );
	
	return 2;
}

static int image_repeat( lua_State * l )
{
	lua::Image * image = (lua::Image *) luaL_checkudata( l, 1, IMAGE_MT );
	
	if ( lua_gettop( l ) == 2 )
	{
		luaL_checktype( l, 2, LUA_TBOOLEAN );
		image->texture->setRepeated( lua_toboolean( l, 2 ) );
	}
	
	lua_pushboolean( l, image->texture->isRepeated() );
	return 1;
}

static int image_rotate( lua_State * l )
{
	lua::Image * image = (lua::Image *) luaL_checkudata( l, 1, IMAGE_MT );
	
	if ( lua_gettop( l ) == 2 ) // rotate
		image->sprite.setRotation( luaL_checknumber( l, 2 ) );
	
	lua_pushnumber( l, image->sprite.getRotation() );
	return 1;
}

// (1) image:scale()
// (2) image:scale( x, y )
// version (2) sets the scale of the image
// both versions return the scale x, y
static int image_scale( lua_State * l )
{
	lua::Image * image = (lua::Image *) luaL_checkudata( l, 1, IMAGE_MT );

	if ( lua_gettop( l ) == 3 ) // setScale
	{
		float x = luaL_checknumber( l, 2 );
		float y = luaL_checknumber( l, 3 );
		
		image->sprite.setScale( x, y );
		
		return 0;
	}

	const sf::Vector2f & scale = image->sprite.getScale();
	
	lua_pushnumber( l, scale.x );
	lua_pushnumber( l, scale.y );
	
	return 2;
}

static int image_size( lua_State * l )
{
	lua::Image * data = (lua::Image *) luaL_checkudata( l, 1, IMAGE_MT );
	sf::Vector2u size = data->texture->getSize();
	
	lua_pushinteger( l, size.x );
	lua_pushinteger( l, size.y );
	
	return 2;
}

static int image_smooth( lua_State * l )
{
	lua::Image * data = (lua::Image *) luaL_checkudata( l, 1, IMAGE_MT );
	
	if ( lua_gettop( l ) == 2 )
	{
		luaL_checktype( l, 2, LUA_TBOOLEAN );
		data->texture->setSmooth( lua_toboolean( l, 2 ) );
	}
	
	lua_pushboolean( l, data->texture->isSmooth() );
	return 1;
}

static int image_subrect( lua_State * l )
{
	lua::Image * data = (lua::Image *) luaL_checkudata( l, 1, IMAGE_MT );
	
	if ( lua_gettop( l ) == 5 )
	{
		int x = luaL_checkinteger( l, 2 );
		int y = luaL_checkinteger( l, 3 );
		int w = luaL_checkinteger( l, 4 );
		int h = luaL_checkinteger( l, 5 );
		
		data->sprite.setTextureRect( sf::IntRect( x, y, w, h ) );
	}
	
	const sf::IntRect & rect = data->sprite.getTextureRect();
	
	lua_pushinteger( l, rect.left );
	lua_pushinteger( l, rect.top );
	lua_pushinteger( l, rect.width );
	lua_pushinteger( l, rect.height );
	
	return 4;
}

static int image_free( lua_State * l )
{
	lua::Image * data = (lua::Image *) luaL_checkudata( l, 1, IMAGE_MT );
	if ( data->display ) hideDrawable( &data->sprite );
	data->~Image();
	return 0;
}

static const struct luaL_Reg libimage_mt [] =
{
	{ "display",	image_display },
	{ "load", 	image_load },
	{ "move",		image_move },
	{ "origin",	image_origin },
	{ "position",	image_position },
	{ "repeat",	image_repeat },
	{ "rotate",	image_rotate },
	{ "scale", 	image_scale },
	{ "smooth",	image_smooth },
	{ "size", 	image_size },
	{ "subrect",	image_subrect },
	{ "__gc", 	image_free },
	{ NULL, 		NULL },
};

/***************************************************************************/

// game.hook( id, fn )
static int game_hook( lua_State * l )
{
	luaL_checktype( l, 1, LUA_TSTRING );
	luaL_checktype( l, 2, LUA_TFUNCTION );
	
	lua_pushvalue( l, 2 );
	addLuaRef( lua_tostring( l, 1 ), luaL_ref( l, LUA_REGISTRYINDEX ) );
	
	return 0;
}

// game.unhook( id )
static int game_unhook( lua_State * l )
{
	removeLuaRef( luaL_checkstring( l, 1 ) );
	return 0;
}

// game.newImage()
// creates a new image userdata
int game_newImage( lua_State * l )
{
	// create userdata and set metatable
	lua::Image * data = (lua::Image *) lua_newuserdata( l, sizeof( lua::Image ) );
	new (data) lua::Image();
	
	luaL_getmetatable( l, IMAGE_MT );
	lua_setmetatable( l, -2 );
	
	data->display = false;
	
	return 1;
}

// game.showText( text [, speaker ] )
// displays a dialogue box
int game_showText( lua_State * l )
{
	bf::showText( luaL_checkstring( l, 1 ), luaL_optstring( l, 2, "" ) );
	return 0;
}

int game_screen( lua_State * l )
{
	lua_pushinteger( l, SCREEN_WIDTH );
	lua_pushinteger( l, SCREEN_HEIGHT );
	return 2;
}

static const struct luaL_Reg libgame[] = 
{
	{ "hook",		game_hook },
	{ "unhook",	game_unhook },
	{ "newImage", 	game_newImage },
	{ "screen",	game_screen },
	{ "showText", 	game_showText },
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
	
	lua_pushvalue( l, 2 );
	
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

// time.date()
// returns the day [1,30], season [0,3], and year in that order
static int time_date( lua_State * l )
{
	const time::Date & date = Time::singleton().getDate();
	lua_pushinteger( l, date.getDay() );
	lua_pushinteger( l, (int) date.getSeason() );
	lua_pushinteger( l, date.getYear() );
	return 3;
}

// time.hour()
// returns the hour [0,23] and minute [0,59]
static int time_hour( lua_State * l )
{
	const time::Hour & hour = Time::singleton().getHour();
	lua_pushinteger( l, hour.get24Hour() );
	lua_pushinteger( l, hour.getMinute() );
	return 2;
}

static const struct luaL_Reg libtime[] =
{
	{ "date", time_date },
	{ "hour",	time_hour },
	{ NULL, NULL },
};

/***************************************************************************/

static std::unordered_map< std::string, int > LuaRef;

void addLuaRef( const std::string & str, int ref )
{
	if ( LuaRef.find( str ) != LuaRef.end() )
		throw Exception( "reference ID already exists" );
	LuaRef.insert( std::make_pair( str, ref ) );
}

void removeLuaRef( const std::string & str )
{
	LuaRef.erase( str );
}

/***************************************************************************/

static lua_State * LUA = nullptr;

void init()
{
	// create lua state
	lua_State * l = LUA = luaL_newstate();
	luaL_openlibs( l );
	
	// texture metatable
	luaL_newmetatable( l, IMAGE_MT );

	lua_pushvalue( l, -1 );
	lua_setfield( l, -2, "__index" );
	
	luaL_setfuncs( l, libimage_mt, 0 );
	lua_pop( l, 1 );
	
	// register custom libraries
	luaL_newlib( l, libgame );
	lua_setglobal( l, "game" );

	luaL_newlib( l, libconsole );
	lua_setglobal( l, "console" );
	
	luaL_newlib( l, libtime );
	lua_setglobal( l, "time" );
}

void cleanup()
{
	for ( auto i : LuaRef )
		luaL_unref( LUA, LUA_REGISTRYINDEX, i.second );
	LuaRef.clear();
	
	lua_close( LUA );
}

lua_State * state()
{
	return LUA;
}

void update( unsigned ms )
{
	for ( auto ref : LuaRef )
	{
		try
		{
			lua_rawgeti( LUA, LUA_REGISTRYINDEX, ref.second );
			lua_pushinteger( LUA, ms );
			
			if ( lua_pcall( LUA, 1, 0, 0 ) )
			{
				std::string err = lua_tostring( LUA, -1 );
				lua_pop( LUA, 1 );
				throw Exception( err );
			}
		}
		catch ( Exception & err )
		{
			Console::singleton() << con::setcerr << err.what() << con::endl;
		}
	}
}

/***************************************************************************/

} // namespace lua

} // namespace bf
