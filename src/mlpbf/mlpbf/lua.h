#pragma once

#include <deque>
#include <lua5.2/lua.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>

namespace bf
{
	namespace lua
	{
		void init();
		void cleanup();
		
		void update( unsigned ms );
	
		lua_State * state();
		
		struct Drawable;

		class Container : public virtual sf::Drawable, public virtual sf::Transformable
		{
			std::deque< lua::Drawable * > m_draw;
			bool m_display;
	
		public:
			Container();
			virtual ~Container();

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

			Drawable();
			virtual ~Drawable() {}
	
			void display( bool state );
	
			virtual const sf::Drawable & getDrawable() const = 0;
			
			int ref;
		};
		
		extern const char * CONTAINER_MT;
		
		extern const char * IMAGE_MT;
		extern const char * TEXT_MT;	
	}
}
