#include "mlpbf/console.h"
#include "mlpbf/console/command.h"
#include "mlpbf/exception.h"
#include "mlpbf/farm.h"
#include "mlpbf/global.h"
#include "mlpbf/lua.h"
#include "mlpbf/player.h"
#include "mlpbf/resource.h"
#include "mlpbf/time.h"

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <deque>
#include <iostream>
#include <unordered_map>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Clock.hpp>

namespace bf
{
namespace lua
{

void addLuaRef( const std::string & str, int ref );
void removeLuaRef( const std::string & str );

sf::Keyboard::Key getKeyFromString( const char * str );
const char * getStringFromKey( sf::Keyboard::Key key );

/***************************************************************************/

const char * CONTAINER_MT = "game.container";
const char * IMAGE_MT = "game.image";
const char * TEXT_MT = "game.text";

Drawable::Drawable() : 
	m_display( false ), 
	m_parent( nullptr ),
	ref( LUA_NOREF )
{
}

void Drawable::display( bool state )
{
	if ( m_display != state )
	{
		if ( state )
			showDrawable( &getDrawable() );
		else
		{
			if ( m_parent )
				m_parent->removeChild( this );
			else
				hideDrawable( &getDrawable() );
		}
		m_display = state;
	}
}

struct Image : public lua::Drawable
{
	res::TexturePtr texture;
	sf::Sprite sprite;
	
	const sf::Sprite & getDrawable() const { return sprite; }
};

struct Text : public lua::Drawable
{
	res::FontPtr font;
	sf::Text text;
	
	const sf::Text & getDrawable() const { return text; }
};

/***************************************************************************/

static int game_fadeIn( lua_State * l )
{
	bf::fadeIn( sf::milliseconds( luaL_checkinteger( l, 1 ) ) );
	return 0;
}

static int game_fadeOut( lua_State * l )
{
	bf::fadeOut( sf::milliseconds( luaL_checkinteger( l, 1 ) ) );
	return 0;
}

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

// game.isKeyPressed( key, ... )
static int game_isKeyPressed( lua_State * l )
{
	int stack = lua_gettop( l );
	for ( int i = 1; i <= stack; i++ )
		lua_pushboolean( l, sf::Keyboard::isKeyPressed( getKeyFromString( lua_tostring( l, i ) ) ) );
	return stack;
}

// game.newContainer()
// creates a new drawing container
static int game_newContainer( lua_State * l )
{
	lua::Container * cnt = (lua::Container *) lua_newuserdata( l, sizeof( lua::Container ) );
	
	luaL_getmetatable( l, CONTAINER_MT );
	lua_setmetatable( l, -2 );
	
	new (cnt) lua::Container();
	
	return 1;
}

// game.newImage()
// creates a new image userdata
static int game_newImage( lua_State * l )
{
	// create userdata and set metatable
	lua::Image * data = (lua::Image *) lua_newuserdata( l, sizeof( lua::Image ) );
	
	luaL_getmetatable( l, IMAGE_MT );
	lua_setmetatable( l, -2 );
	
	new (data) lua::Image();
	
	return 1;
}

static int game_newText( lua_State * l )
{
	lua::Text * data = (lua::Text *) lua_newuserdata( l, sizeof( lua::Text ) );
		
	luaL_getmetatable( l, TEXT_MT );
	lua_setmetatable( l, -2 );
	
	new (data) lua::Text();
	
	return 1;
}

// game.showText( text [, speaker ] )
// displays a dialogue box
static int game_showText( lua_State * l )
{
	bf::showText( luaL_checkstring( l, 1 ), luaL_optstring( l, 2, "" ) );
	return 0;
}

static int game_screen( lua_State * l )
{
	lua_pushinteger( l, SCREEN_WIDTH );
	lua_pushinteger( l, SCREEN_HEIGHT );
	return 2;
}

static const struct luaL_Reg libgame[] = 
{
	{ "fadeIn",		game_fadeIn },
	{ "fadeOut",		game_fadeOut },
	{ "hook",			game_hook },
	{ "unhook",		game_unhook },
	{ "isKeyPressed",	game_isKeyPressed },
	{ "newContainer",	game_newContainer },
	{ "newImage", 		game_newImage },
	{ "newText",		game_newText },
	{ "screen",		game_screen },
	{ "showText", 		game_showText },
	{ NULL, 			NULL },
};

/***************************************************************************/

// console.write( str [, col ] )
// prints a line to the console
static int console_write( lua_State * l )
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
static int console_execute( lua_State * l )
{
	Console::singleton().execute( luaL_checkstring( l, 1 ) );
	return 0;
}

static int console_hook( lua_State * l )
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

// (1) player.animate( str )
static int player_animate( lua_State * l )
{
	Player::singleton().animate( luaL_checkstring( l, 1 ) );
	return 0;
}

// (1) player.collision( bool )
// switches on/off collision detection for the player
static int player_collision( lua_State * l )
{
	luaL_checktype( l, 1, LUA_TBOOLEAN );
	Player::singleton().enableCollision( lua_toboolean( l, 1 ) );
	return 0;
}

static int player_control( lua_State * l )
{
	luaL_checktype( l, 1, LUA_TBOOLEAN );
	Player::singleton().enableControl( lua_toboolean( l, 1 ) );
	return 0;
}

// (1) player.position()
// (2) player.position( x, y )
// (3) player.position( map, x, y )
static int player_position( lua_State * l )
{
	Player & player = Player::singleton();
	
	int n = lua_gettop( l );
	if ( n == 2 )
		player.setPosition( sf::Vector2f( luaL_checknumber( l, 1 ), luaL_checknumber( l, 2 ) ) );
	else if ( n == 3 )
		player.setMap( luaL_checkstring( l, 1 ), sf::Vector2f( luaL_checknumber( l, 2 ), luaL_checknumber( l, 3 ) ) );
	
	const sf::Vector2f & pos = player.getPosition();
	
	lua_pushnumber( l, pos.x );
	lua_pushnumber( l, pos.y );
	
	return 2;
}

static const struct luaL_Reg libplayer[] =
{
	{ "animate", 			player_animate },
	{ "enableCollision", 	player_collision },
	{ "enableControl", 		player_control },
	{ "position", 			player_position },
	{ NULL, 				NULL },
};

/***************************************************************************/

// (1) time.date()
// (2) time.date( inc )
// (3) time.date( str )
// (4) time.date( season, day [, year ] )
// returns the day [1,30], season [0,3], and year in that order
static int time_date( lua_State * l )
{
	time::Date & date = Time::singleton().getDate();
	
	int n = lua_gettop( l );
	if ( n == 1 )
	{
		if ( lua_isnumber( l, 1 ) )
			date.increment( lua_tointeger( l, 1 ) );
		else
			date.set( lua_tostring( l, 1 ) );
	}
	else if ( n == 2 )
	{
		int s = luaL_checkinteger( l, 1 );
		int d = luaL_checkinteger( l, 2 );
		int y = luaL_optint( l, 3, 0 );
		
		luaL_argcheck( l, 0 <= s && s <= 3, 1, "season must be between 0 and 3" );
		
		date.set( (time::Season) s, d, y );
	}
	
	lua_pushinteger( l, date.getDay() );
	lua_pushinteger( l, (int) date.getSeason() );
	lua_pushinteger( l, date.getYear() );
	return 3;
}

// (1) time.hour()
// (2) time.hour( inc )
// (3) time.hour( str )
// (4) time.hour( h, m )
// returns the hour [0,23] and minute [0,59]
// BUG: incrementing past midnight doesn't change the day
static int time_hour( lua_State * l )
{
	time::Hour & hour = Time::singleton().getHour();
	
	int n = lua_gettop( l );
	if ( n == 1 )
	{
		if ( lua_isnumber( l, 1 ) )
			hour.increment( lua_tointeger( l, 1 ) );
		else
			hour.set( lua_tostring( l, 1 ) );
	}
	else if ( n == 2 )
		hour.set( luaL_checkinteger( l, 1 ), luaL_checkinteger( l, 2 ) );
	
	lua_pushinteger( l, hour.get24Hour() );
	lua_pushinteger( l, hour.getMinute() );
	return 2;
}

// (1) time.state()
// (2) time.state( bool )
static int time_state( lua_State * l )
{
	Time & time = Time::singleton();
	
	if ( lua_gettop( l ) == 1 )
	{
		luaL_checktype( l, 1, LUA_TBOOLEAN );
		time.setState( lua_toboolean( l, 1 ) );
	}
		
	lua_pushboolean( l, time.getState() );

	return 1;
}

static int time_timescale( lua_State * l )
{
	Time::singleton().setTimescale( luaL_checknumber( l, 1 ) );
	return 0;
}

static const struct luaL_Reg libtime[] =
{
	{ "date", 	time_date },
	{ "hour",		time_hour },
	{ "state", 	time_state },
	{ "timescale", time_timescale },
	{ NULL, 		NULL },
};

/***************************************************************************/

static const char * TIMER_MT = "game.timer";

static int timer_new( lua_State * l )
{
	sf::Clock * timer = (sf::Clock *) lua_newuserdata( l, sizeof( sf::Clock ) );
	
	luaL_getmetatable( l, TIMER_MT );
	lua_setmetatable( l, -2 );
	
	new (timer) sf::Clock();
	
	return 1;
}

static int timer_free( lua_State * l )
{
	sf::Clock * timer = (sf::Clock *) luaL_checkudata( l, 1, TIMER_MT );
	timer->~Clock();
	return 0;
}

static int timer_getElapsedTime( lua_State * l )
{
	sf::Clock * timer = (sf::Clock *) luaL_checkudata( l, 1, TIMER_MT );
	lua_pushinteger( l, timer->getElapsedTime().asMilliseconds() );
	return 1;
}

static int timer_restart( lua_State * l )
{
	sf::Clock * timer = (sf::Clock *) luaL_checkudata( l, 1, TIMER_MT );
	lua_pushinteger( l, timer->restart().asMilliseconds() );
	return 1;
}

static const struct luaL_Reg libtimer [] =
{
	{ "new",	timer_new },
	{ NULL, 	NULL },
};

static const struct luaL_Reg libtimer_mt [] =
{
	{ "getElapsedTime",		timer_getElapsedTime },
	{ "restart",			timer_restart },
	{ "__gc",				timer_free },
	{ NULL, 				NULL },
};

/***************************************************************************/

lua::Container::Container() : 
	m_display( false )
{
}

lua::Container::~Container()
{
	display( false );
	for ( lua::Drawable * d : m_draw )
	{
		luaL_unref( lua::state(), LUA_REGISTRYINDEX, d->ref );
		d->ref = LUA_NOREF;
	}
}

void lua::Container::addChild( lua::Drawable * d )
{
	m_draw.push_back( d );
	d->m_parent = this;
}

void lua::Container::removeChild( const lua::Drawable * d )
{
	auto find = std::find( m_draw.begin(), m_draw.end(), d );
	if ( find != m_draw.end() )
	{
		(*find)->m_parent = nullptr;
		(*find)->m_display = false;
		m_draw.erase( find );
	}
}

void lua::Container::display( bool state )
{
	if ( m_display != state )
	{
		if ( ( m_display = state ) )
			showDrawable( this );
		else
			hideDrawable( this );
			
		for ( lua::Drawable * d : m_draw )
			d->m_display = m_display;
	}
}

void lua::Container::draw( sf::RenderTarget & target, sf::RenderStates states ) const
{
	states.transform *= getTransform();
	for ( const lua::Drawable * d : m_draw )
		target.draw( d->getDrawable(), states );
}

static int container_addImage( lua_State * l )
{
	lua::Container * container = (lua::Container *) luaL_checkudata( l, 1, CONTAINER_MT );
	lua::Image * image = (lua::Image *) luaL_checkudata( l, 2, IMAGE_MT );
	
	container->addChild( image );
	
	if ( image->ref == LUA_NOREF )
	{
		lua_pushvalue( l, 2 );
		image->ref = luaL_ref( l, LUA_REGISTRYINDEX );
	}

	return 0;
}

static int container_addText( lua_State * l )
{
	lua::Container * container = (lua::Container *) luaL_checkudata( l, 1, CONTAINER_MT );
	lua::Text * text = (lua::Text *) luaL_checkudata( l, 2, TEXT_MT );
	
	container->addChild( text );
	
	if ( text->ref == LUA_NOREF )
	{
		lua_pushvalue( l, 2 );
		text->ref = luaL_ref( l, LUA_REGISTRYINDEX );
	}
	
	return 0;
}

static int container_display( lua_State * l )
{
	lua::Container * container = (lua::Container *) luaL_checkudata( l, 1, CONTAINER_MT );
	luaL_checktype( l, 2, LUA_TBOOLEAN );
	
	container->display( lua_toboolean( l, 2 ) );
	
	return 0;
}

static int container_origin( lua_State * l )
{
	lua::Container * container = (lua::Container *) luaL_checkudata( l, 1, CONTAINER_MT );
	
	if ( lua_gettop( l ) == 3 )
	{
		float x = luaL_checknumber( l, 2 );
		float y = luaL_checknumber( l, 3 );
		
		container->setOrigin( x, y );
	}
	
	const sf::Vector2f & origin = container->getOrigin();
	lua_pushnumber( l, origin.x );
	lua_pushnumber( l, origin.y );
	
	return 2;
}

static int container_position( lua_State * l )
{
	lua::Container * container = (lua::Container *) luaL_checkudata( l, 1, CONTAINER_MT );
	
	if ( lua_gettop( l ) == 3 )
	{
		float x = luaL_checknumber( l, 2 );
		float y = luaL_checknumber( l, 3 );
		
		container->setPosition( x, y );
	}

	const sf::Vector2f & pos = container->getPosition();
	lua_pushnumber( l, pos.x );
	lua_pushnumber( l, pos.y );
	
	return 2;
}

static int container_removeImage( lua_State * l )
{
	lua::Container * container = (lua::Container *) luaL_checkudata( l, 1, CONTAINER_MT );
	lua::Image * image = (lua::Image *) luaL_checkudata( l, 2, IMAGE_MT );
	
	container->removeChild( image );
	
	luaL_unref( l, LUA_REGISTRYINDEX, image->ref );
	image->ref = LUA_NOREF;

	return 0;
}

static int container_removeText( lua_State * l )
{
	lua::Container * container = (lua::Container *) luaL_checkudata( l, 1, CONTAINER_MT );
	lua::Text * text = (lua::Text *) luaL_checkudata( l, 2, TEXT_MT );
	
	container->removeChild( text );
	
	luaL_unref( l, LUA_REGISTRYINDEX, text->ref );
	text->ref = LUA_NOREF;

	return 0;
}

static int container_free( lua_State * l )
{
	lua::Container * container = (lua::Container *) luaL_checkudata( l, 1, CONTAINER_MT );
	container->~Container();
	return 0;
}

static const struct luaL_Reg libcontainer_mt[] =
{
	{ "addImage",		container_addImage },
	{ "addText",		container_addText },
	{ "display",		container_display },
	{ "origin",		container_origin },
	{ "position",		container_position },
	{ "removeImage", 	container_removeImage },
	{ "removeText",	container_removeText },
	{ "__gc",			container_free },
	{ NULL, 			NULL },
};

/***************************************************************************/

static int image_color( lua_State * l )
{
	lua::Image * image = (lua::Image *) luaL_checkudata( l, 1, IMAGE_MT );
	
	if ( lua_gettop( l ) >= 4 )
	{
		int r = luaL_checkinteger( l, 2 );
		int g = luaL_checkinteger( l, 3 );
		int b = luaL_checkinteger( l, 4 );
		int a = luaL_optinteger( l, 5, 255 );
		
		image->sprite.setColor( sf::Color( r, g, b, a ) );
	}
	
	const sf::Color & color = image->sprite.getColor();
	
	lua_pushinteger( l, color.r );
	lua_pushinteger( l, color.g );
	lua_pushinteger( l, color.b );
	lua_pushinteger( l, color.a );

	return 4;
}

static int image_display( lua_State * l )
{
	lua::Image * image = (lua::Image *) luaL_checkudata( l, 1, IMAGE_MT );
	luaL_checktype( l, 2, LUA_TBOOLEAN );
	
	image->display( lua_toboolean( l, 2 ) );
	
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
	data->display( false );
	data->~Image();
	return 0;
}

static const struct luaL_Reg libimage_mt [] =
{
	{ "color",	image_color },
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

static int text_color( lua_State * l )
{
	lua::Text * text = (lua::Text *) luaL_checkudata( l, 1, TEXT_MT );
	
	if ( lua_gettop( l ) >= 4 )
	{
		int r = luaL_checkinteger( l, 2 );
		int g = luaL_checkinteger( l, 3 );
		int b = luaL_checkinteger( l, 4 );
		int a = luaL_optinteger( l, 5, 255 );
		
		text->text.setColor( sf::Color( r, g, b, a ) );
	}
	
	const sf::Color & color = text->text.getColor();
	
	lua_pushinteger( l, color.r );
	lua_pushinteger( l, color.g );
	lua_pushinteger( l, color.b );
	lua_pushinteger( l, color.a );

	return 4;
}

static int text_display( lua_State * l )
{
	lua::Text * text = (lua::Text *) luaL_checkudata( l, 1, TEXT_MT );
	luaL_checktype( l, 2, LUA_TBOOLEAN );
	
	text->display( lua_toboolean( l, 2 ) );
	
	return 0;
}

static int text_load( lua_State * l )
{
	lua::Text * text = (lua::Text *) luaL_checkudata( l, 1, TEXT_MT );
	
	text->font = res::loadFont( luaL_checkstring( l, 2 ) );
	text->text.setFont( *text->font );
	
	return 0;
}

static int text_origin( lua_State * l )
{
	lua::Text * text = (lua::Text *) luaL_checkudata( l, 1, TEXT_MT );
	
	if ( lua_gettop( l ) == 3 )
	{
		float x = luaL_checknumber( l, 2 );
		float y = luaL_checknumber( l, 3 );
		
		text->text.setOrigin( x, y );
	}
	
	const sf::Vector2f & origin = text->text.getOrigin();
	lua_pushnumber( l, origin.x );
	lua_pushnumber( l, origin.y );
	
	return 2;
}

static int text_position( lua_State * l )
{
	lua::Text * text = (lua::Text *) luaL_checkudata( l, 1, TEXT_MT );
	
	if ( lua_gettop( l ) == 3 )
	{
		float x = luaL_checknumber( l, 2 );
		float y = luaL_checknumber( l, 3 );
		
		text->text.setPosition( x, y );
	}

	const sf::Vector2f & pos = text->text.getPosition();
	lua_pushnumber( l, pos.x );
	lua_pushnumber( l, pos.y );
	
	return 2;
}

static int text_rotate( lua_State * l )
{
	lua::Text * text = (lua::Text *) luaL_checkudata( l, 1, TEXT_MT );
	
	if ( lua_gettop( l ) == 2 ) // rotate
		text->text.setRotation( luaL_checknumber( l, 2 ) );
	
	lua_pushnumber( l, text->text.getRotation() );
	return 1;
}

static int text_scale( lua_State * l )
{
	lua::Text * text = (lua::Text *) luaL_checkudata( l, 1, TEXT_MT );

	if ( lua_gettop( l ) == 3 ) // setScale
	{
		float x = luaL_checknumber( l, 2 );
		float y = luaL_checknumber( l, 3 );
		
		text->text.setScale( x, y );
		
		return 0;
	}

	const sf::Vector2f & scale = text->text.getScale();
	
	lua_pushnumber( l, scale.x );
	lua_pushnumber( l, scale.y );
	
	return 2;
}

static int text_size( lua_State * l )
{
	lua::Text * text = (lua::Text *) luaL_checkudata( l, 1, TEXT_MT );
	sf::FloatRect bounds = text->text.getGlobalBounds();
	
	lua_pushnumber( l, bounds.width );
	lua_pushnumber( l, bounds.height );
	
	return 2;
}

static int text_charsize( lua_State * l )
{
	lua::Text * text = (lua::Text *) luaL_checkudata( l, 1, TEXT_MT );
	
	if ( lua_gettop( l ) == 2 )
		text->text.setCharacterSize( luaL_checkinteger( l, 2 ) );
		
	lua_pushinteger( l, text->text.getCharacterSize() );
	return 1;
}

static int text_string( lua_State * l )
{
	lua::Text * text = (lua::Text *) luaL_checkudata( l, 1, TEXT_MT );
	
	if ( lua_gettop( l ) == 2 )
		text->text.setString( luaL_checkstring( l, 2 ) );
		
	lua_pushstring( l, text->text.getString().toAnsiString().c_str() );
	return 1;
}

static int text_free( lua_State * l )
{
	lua::Text * data = (lua::Text *) luaL_checkudata( l, 1, TEXT_MT );
	data->display( false );
	data->~Text();
	return 0;
}

static const struct luaL_Reg libtext_mt [] =
{
	{ "charsize",	text_charsize },
	{ "color",	text_color },
	{ "display",	text_display },
	{ "load",		text_load },
	{ "origin",	text_origin },
	{ "position",	text_position },
	{ "rotate",	text_rotate },
	{ "scale",	text_scale },
	{ "size",		text_size },
	{ "string",	text_string },
	{ "__gc", 	text_free },
	{ NULL, 		NULL },
};

/***************************************************************************/

static const char * FIELDTILE_MT = "field.tile";

static int lua_field_getTile( lua_State * l )
{
	using namespace farm;

	int x = luaL_checkinteger( l, 1 );
	int y = luaL_checkinteger( l, 2 );
	
	luaL_argcheck( l, 0 < x && x <= field::WIDTH, 1, "tile out of bounds" );
	luaL_argcheck( l, 0 < y && y <= field::HEIGHT, 2, "tile out of bounds" );
	
	field::Tile & ftile = field::getTile( x - 1, y - 1 );

	field::Tile ** tile = (field::Tile **) lua_newuserdata( l, sizeof( field::Tile * ) );
	*tile = &ftile;
	
	luaL_setmetatable( l, FIELDTILE_MT );
	
	return 1;
}

static int lua_field_size( lua_State * l )
{
	lua_pushinteger( l, farm::field::WIDTH );
	lua_pushinteger( l, farm::field::HEIGHT );
	return 2;
}

static const struct luaL_Reg libfield [] =
{
	{ "getTile", 	lua_field_getTile },
	{ "size", 	lua_field_size },
	{ NULL, 		NULL },
};

static int lua_field_tile_till( lua_State * l )
{
	using namespace farm;

	field::Tile ** ltile = (field::Tile **) luaL_checkudata( l, 1, FIELDTILE_MT );
	field::Tile & tile = **ltile;

	if ( lua_gettop( l ) == 2 )
		tile.till = luaL_checkinteger( l, 2 );
	
	lua_pushinteger( l, tile.till );
	
	return 1;
}

static int lua_field_tile_water( lua_State * l )
{
	using namespace farm;

	field::Tile ** ltile = (field::Tile **) luaL_checkudata( l, 1, FIELDTILE_MT );
	field::Tile & tile = **ltile;
	
	if ( lua_gettop( l ) == 2 )
		tile.water = lua_toboolean( l, 2 );
		
	lua_pushinteger( l, tile.water );

	return 1;
}

static const struct luaL_Reg libfieldtile_mt [] =
{
	{ "till",		lua_field_tile_till },
	{ "water",	lua_field_tile_water },
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

inline void register_metatable( lua_State * l, const char * name, const struct luaL_Reg lib[] )
{
	luaL_newmetatable( l, name );
	
	lua_pushvalue( l, -1 );
	lua_setfield( l, -2, "__index" );
	
	luaL_setfuncs( l, lib, 0 );
	lua_pop( l, 1 );
}

// Macro because inline function throws a warning
#define register_library(L,n,l) (luaL_newlib(L,l),lua_setglobal(L,n))

void init()
{
	// create lua state
	lua_State * l = LUA = luaL_newstate();
	luaL_openlibs( l );
	
	// register metatables
	register_metatable( l, CONTAINER_MT, libcontainer_mt );
	register_metatable( l, IMAGE_MT, libimage_mt );
	register_metatable( l, TEXT_MT, libtext_mt );
	register_metatable( l, TIMER_MT, libtimer_mt );
	register_metatable( l, FIELDTILE_MT, libfieldtile_mt );
	
	// register custom libraries
	register_library( l, "game", libgame );
	register_library( l, "console", libconsole );
	register_library( l, "time", libtime );
	register_library( l, "player", libplayer );
	register_library( l, "timer", libtimer );
	register_library( l, "field", libfield );
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
			Console::singleton() << con::setcerr << "Unhooking lua function " << ref.first << con::endl;
			removeLuaRef( ref.first );
		}
	}
}

void save_rec( FILE * fp )
{
	assert( lua_istable( LUA, -1 ) );

	std::uint8_t type;
	lua_pushnil( LUA );
	
	while ( lua_next( LUA, -2 ) )
	{
		switch ( type = lua_type( LUA, -1 ) )
		{
			case LUA_TBOOLEAN:
			{
				fputs( lua_tostring( LUA, -2 ), fp ); fputc( '\0', fp );
				fwrite( &type, sizeof( std::uint8_t ), 1, fp );
				bool b = lua_toboolean( LUA, -1 );
				fwrite( &b, sizeof( bool ), 1, fp );
			}
			break;
			case LUA_TNUMBER:
			{
				fputs( lua_tostring( LUA, -2 ), fp ); fputc( '\0', fp );
				fwrite( &type, sizeof( std::uint8_t ), 1, fp );
				LUA_NUMBER num = lua_tonumber( LUA, -1 );
				fwrite( &num, sizeof( LUA_NUMBER ), 1, fp );
			}
			break;
			case LUA_TSTRING:
				fputs( lua_tostring( LUA, -2 ), fp ); fputc( '\0', fp );
				fwrite( &type, sizeof( std::uint8_t ), 1, fp );
				fputs( lua_tostring( LUA, -1 ), fp ); fputc( '\0', fp );
			break;
			case LUA_TTABLE:
				fputs( lua_tostring( LUA, -2 ), fp ); fputc( '\0', fp );
				fwrite( &type, sizeof( std::uint8_t ), 1, fp );
				save_rec( fp );
				fputc( '\0', fp );
			break;
			default: break;
		}
		lua_pop( LUA, 1 );
	}
	
	lua_pop( LUA, 1 );
}

void save( FILE * fp )
{
	lua_getglobal( LUA, "data" );
	save_rec( fp );
	fputc( '\0', fp );
}

void load_rec( FILE * fp )
{
	assert( lua_istable( LUA, -1 ) );

	std::uint8_t type;
	
	while ( true )
	{
		// get the key name
		std::string key;
		for ( char c = fgetc( fp ); c != '\0'; c = fgetc( fp ) )
			key.append( 1, c );
			
		// get the value type
		fread( &type, sizeof( std::uint8_t ), 1, fp );
		
		// push the key-value pair to the table
		switch ( type )
		{
			case LUA_TBOOLEAN:
			{
				bool b; fread( &b, sizeof( bool ), 1, fp );
				lua_pushboolean( LUA, b );
				lua_setfield( LUA, -2, key.c_str() );
			}
			break;
			case LUA_TNUMBER:
			{
				LUA_NUMBER num; fread( &num, sizeof( LUA_NUMBER ), 1, fp );
				lua_pushnumber( LUA, num );
				lua_setfield( LUA, -2, key.c_str() );
			}
			break;
			case LUA_TSTRING:
			{
				std::string val; 
				for ( char c = fgetc( fp ); c != '\0'; c = fgetc( fp ) ) 
					val.append( 1, c );
				lua_pushstring( LUA, val.c_str() );
				lua_setfield( LUA, -2, key.c_str() );
			}
			break;
			case LUA_TTABLE:
				fputs( lua_tostring( LUA, -2 ), fp ); fputc( '\0', fp );
				fwrite( &type, sizeof( std::uint8_t ), 1, fp );
				save_rec( fp );
			break;
			default: break;
		}
		
		// check if reached end-of-table with null terminator
		char c = fgetc( fp );
		if ( c == '\0' ) return;
		
		// unget the character and repeat
		ungetc( c, fp );
	}
}

void load( FILE * fp )
{
	lua_newtable( LUA );
	load_rec( fp );
	lua_setglobal( LUA, "data" );
}

/***************************************************************************/

static const struct { const char * str; sf::Keyboard::Key key; } keyList[] =
{
	{ "a",		sf::Keyboard::A },
	{ "b",		sf::Keyboard::B },
	{ "c",		sf::Keyboard::C },
	{ "d",		sf::Keyboard::D },
	{ "e",		sf::Keyboard::E },
	{ "f",		sf::Keyboard::F },
	{ "g",		sf::Keyboard::G },
	{ "h",		sf::Keyboard::H },
	{ "i",		sf::Keyboard::I },
	{ "j",		sf::Keyboard::J },
	{ "k",		sf::Keyboard::K },
	{ "l",		sf::Keyboard::L },
	{ "m",		sf::Keyboard::M },
	{ "n",		sf::Keyboard::N },
	{ "o",		sf::Keyboard::O },
	{ "p",		sf::Keyboard::P },
	{ "q",		sf::Keyboard::Q },
	{ "r",		sf::Keyboard::R },
	{ "s",		sf::Keyboard::S },
	{ "t",		sf::Keyboard::T },
	{ "u",		sf::Keyboard::U },
	{ "v",		sf::Keyboard::V },
	{ "w",		sf::Keyboard::W },
	{ "x",		sf::Keyboard::X },
	{ "y",		sf::Keyboard::Y },
	{ "z",		sf::Keyboard::Z },
	{ "num0",		sf::Keyboard::Num0 },
	{ "num1",		sf::Keyboard::Num1 },
	{ "num2",		sf::Keyboard::Num2 },
	{ "num3",		sf::Keyboard::Num3 },
	{ "num4",		sf::Keyboard::Num4 },
	{ "num5",		sf::Keyboard::Num5 },
	{ "num6",		sf::Keyboard::Num6 },
	{ "num7",		sf::Keyboard::Num7 },
	{ "num8",		sf::Keyboard::Num8 },
	{ "num9",		sf::Keyboard::Num9 },
	{ "escape",	sf::Keyboard::Escape },
	{ "lcontrol",	sf::Keyboard::LControl },
	{ "lshift",	sf::Keyboard::LShift },
	{ "lalt",		sf::Keyboard::LAlt },
	{ "lsystem",	sf::Keyboard::LSystem },
	{ "rcontrol",	sf::Keyboard::RControl },
	{ "rshift",	sf::Keyboard::RShift },
	{ "ralt",		sf::Keyboard::RAlt },
	{ "rsystem",	sf::Keyboard::RSystem },
	{ "menu",		sf::Keyboard::Menu },
	{ "lbracket",	sf::Keyboard::LBracket },
	{ "rbracket",	sf::Keyboard::RBracket },
	{ "semicolon",	sf::Keyboard::SemiColon },
	{ "comma",	sf::Keyboard::Comma },
	{ "period",	sf::Keyboard::Period },
	{ "quote",	sf::Keyboard::Quote },
	{ "slash",	sf::Keyboard::Slash },
	{ "backslash",	sf::Keyboard::BackSlash },
	{ "tilde",	sf::Keyboard::Tilde },
	{ "equal",	sf::Keyboard::Equal },
	{ "dash",		sf::Keyboard::Dash },
	{ "space",	sf::Keyboard::Space },
	{ "return",	sf::Keyboard::Return },
	{ "backspace",	sf::Keyboard::BackSpace },
	{ "tab",		sf::Keyboard::Tab },
	{ "pageup",	sf::Keyboard::PageUp },
	{ "pagedown",	sf::Keyboard::PageDown },
	{ "end",		sf::Keyboard::End },
	{ "home",		sf::Keyboard::Home },
	{ "insert",	sf::Keyboard::Insert },
	{ "delete",	sf::Keyboard::Delete },
	{ "add",		sf::Keyboard::Add },
	{ "subtract",	sf::Keyboard::Subtract },
	{ "multiply",	sf::Keyboard::Multiply },
	{ "divide",	sf::Keyboard::Divide },
	{ "left",		sf::Keyboard::Left },
	{ "right",	sf::Keyboard::Right },
	{ "up",		sf::Keyboard::Up },
	{ "down",		sf::Keyboard::Down },
	{ "numpad0",	sf::Keyboard::Numpad0 },
	{ "numpad1",	sf::Keyboard::Numpad1 },
	{ "numpad2",	sf::Keyboard::Numpad2 },
	{ "numpad3",	sf::Keyboard::Numpad3 },
	{ "numpad4",	sf::Keyboard::Numpad4 },
	{ "numpad5",	sf::Keyboard::Numpad5 },
	{ "numpad6",	sf::Keyboard::Numpad6 },
	{ "numpad7",	sf::Keyboard::Numpad7 },
	{ "numpad8",	sf::Keyboard::Numpad8 },
	{ "numpad9",	sf::Keyboard::Numpad9 },
	{ "f1",		sf::Keyboard::F1 },
	{ "f2",		sf::Keyboard::F2 },
	{ "f3",		sf::Keyboard::F3 },
	{ "f4",		sf::Keyboard::F4 },
	{ "f5",		sf::Keyboard::F5 },
	{ "f6",		sf::Keyboard::F6 },
	{ "f7",		sf::Keyboard::F7 },
	{ "f8",		sf::Keyboard::F8 },
	{ "f9",		sf::Keyboard::F9 },
	{ "f10",		sf::Keyboard::F10 },
	{ "f11",		sf::Keyboard::F11 },
	{ "f12",		sf::Keyboard::F12 },
	{ "f13",		sf::Keyboard::F13 },
	{ "f14",		sf::Keyboard::F14 },
	{ "f15",		sf::Keyboard::F15 },
	{ "pause",	sf::Keyboard::Pause },
	{ NULL,		sf::Keyboard::Unknown },
};

sf::Keyboard::Key getKeyFromString( const char * str )
{	
	for ( int i = 0; keyList[i].str != NULL; i++ )
		if ( strcmp( str, keyList[i].str ) == 0 )
			return keyList[i].key;
	return sf::Keyboard::Unknown;
}

const char * getStringFromKey( sf::Keyboard::Key key )
{
	for ( int i = 0; keyList[i].str != NULL; i++ )
		if ( key == keyList[i].key )
			return keyList[i].str;
	return NULL;
}

/***************************************************************************/

} // namespace lua

} // namespace bf
