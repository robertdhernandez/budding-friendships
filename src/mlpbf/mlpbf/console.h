#pragma once

#include "utility/listener/key.h"
#include "utility/listener/text.h"
#include "console/command.h"

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/NonCopyable.hpp>

namespace bf
{
	class Console : 
		public sf::Drawable,
		public util::KeyListener, 
		public util::TextListener, 
		private sf::NonCopyable
	{
	public:
		static Console& singleton();
		static const sf::Keyboard::Key KEY = sf::Keyboard::Tab;

		void state( bool state ) { m_active = state; }
		bool state() const { return m_active; }

		operator bool() const { return state(); }

		void addCommand( con::Command& );
		const std::vector< con::Command* >& getCommands() const { return m_cmds; }

		enum
		{
			DEFAULT_COLOR = 0xFFFFFF,
			ERROR_COLOR   = 0xFF0000,
			INFO_COLOR    = 0x1C56D4
		};

		void pushLine( const std::string& line, unsigned color = DEFAULT_COLOR );

		void clearHistory();
		void clearCommands();

	public:
		template< typename T >
		friend Console & operator<<( Console &, T );
	
		void setBufferColor( int c ) { m_bufferColor = c; }
		void pushBuffer();

	private:
		Console();

		void execute( const std::string& );

		void onKeyPressed( const sf::Event::KeyEvent& );
		void onKeyReleased( const sf::Event::KeyEvent& );
		void onTextEntered( const sf::Event::TextEvent& );

		void draw( sf::RenderTarget&, sf::RenderStates ) const;

	private:
		bool m_active;
		int m_index;

		std::shared_ptr< sf::Font > m_font;

		std::string m_input;
		std::vector< std::pair< std::string, int > > m_history;

		int m_bufferOffset, m_bufferColor;
		std::ostringstream m_buffer;

		std::vector< con::Command* > m_cmds;
	};
	
	template< typename T >
	Console & operator<<( Console & c, T t )
	{
		c.m_buffer << t;
		return c;
	}

	template<>
	inline Console& operator<<( Console & c, void (*fn)( Console& ) )
	{
		(*fn)( c );
		return c;
	}

	namespace con
	{
		void endl( Console& );

		void setcdef( Console& );  // Default color 
		void setcerr( Console& );  // Error color
		void setcinfo( Console& ); // Info color
	}
}
