#include "mlpbf/console.h"
#include "mlpbf/console/command.h"
#include "mlpbf/exception.h"
#include "mlpbf/global.h"
#include "mlpbf/lua.h"
#include "mlpbf/resource.h"
#include "mlpbf/time.h"

#include <algorithm>
#include <cstring>
#include <deque>
#include <iostream>
#include <unordered_map>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

namespace bf
{
namespace lua
{

void addLuaRef( const std::string & str, int ref );
void removeLuaRef( const std::string & str );

/***************************************************************************/

struct Drawable;

class Container : public sf::Drawable, public sf::Transformable
{
	std::deque< lua::Drawable * > m_draw;
	bool m_display;
	
public:
	Container();
	~Container();

	void addChild( lua::Drawable * d );
	void removeChild( const lua::Drawable * d );
	
	void display( bool state );
	
	void draw( sf::RenderTarget & target, sf::RenderStates states ) const;
};

class Drawable
{
	bool m_display;
	Container * m_parent;

public:
	friend class Container;

	Drawable() : 
		m_display( false ), 
		m_parent( nullptr ) 
	{
	}
	
	virtual ~Drawable()
	{
		display( false );
	}
	
	void display( bool state )
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
	
	virtual const sf::Drawable & getDrawable() const = 0;
};

static const char * CONTAINER_MT = "game.container";

struct Image : public lua::Drawable
{
	res::TexturePtr texture;
	sf::Sprite sprite;
	
	const sf::Sprite & getDrawable() const { return sprite; }
};

static const char * IMAGE_MT = "game.image";

struct Text : public lua::Drawable
{
	res::FontPtr font;
	sf::Text text;
	
	const sf::Text & getDrawable() const { return text; }
};

static const char * TEXT_MT = "game.text";

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

// game.newContainer()
// creates a new drawing container
static int game_newContainer( lua_State * l )
{
	lua::Container * cnt = (lua::Container *) lua_newuserdata( l, sizeof( lua::Container ) );
	new (cnt) lua::Container();
	
	luaL_getmetatable( l, CONTAINER_MT );
	lua_setmetatable( l, -2 );
	
	return 1;
}

// game.newImage()
// creates a new image userdata
static int game_newImage( lua_State * l )
{
	// create userdata and set metatable
	lua::Image * data = (lua::Image *) lua_newuserdata( l, sizeof( lua::Image ) );
	new (data) lua::Image();
	
	luaL_getmetatable( l, IMAGE_MT );
	lua_setmetatable( l, -2 );
	
	return 1;
}

static int game_newText( lua_State * l )
{
	lua::Text * data = (lua::Text *) lua_newuserdata( l, sizeof( lua::Text ) );
	new (data) lua::Text();
	
	luaL_getmetatable( l, TEXT_MT );
	lua_setmetatable( l, -2 );
	
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
	{ "hook",			game_hook },
	{ "unhook",		game_unhook },
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

lua::Container::Container() : 
	m_display( false )
{
}

lua::Container::~Container()
{
	display( false );
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
		if ( m_display = state )
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

	return 0;
}

static int container_addText( lua_State * l )
{
	lua::Container * container = (lua::Container *) luaL_checkudata( l, 1, CONTAINER_MT );
	lua::Text * text = (lua::Text *) luaL_checkudata( l, 2, TEXT_MT );
	
	container->addChild( text );
	
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

	return 0;
}

static int container_removeText( lua_State * l )
{
	lua::Container * container = (lua::Container *) luaL_checkudata( l, 1, CONTAINER_MT );
	lua::Text * text = (lua::Text *) luaL_checkudata( l, 2, TEXT_MT );
	
	container->removeChild( text );

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
	
	// register custom libraries
	register_library( l, "game", libgame );
	register_library( l, "console", libconsole );
	register_library( l, "time", libtime );
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

/***************************************************************************/

} // namespace lua

} // namespace bf
