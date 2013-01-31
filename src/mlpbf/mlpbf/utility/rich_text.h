#pragma once

#include "../resource/loader/font.h"

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>

#include <sstream>
#include <tuple>
#include <vector>

namespace bf
{
	namespace util
	{
		// A modifier is determined by anything inside two braces: { and }
		// Arguments can be set using a colon following comma separated values
		// The inputted string does NOT include the braces
		// Examples: 
		//	{b}			 = ("b",{})
		//	{c:50,50,50} = ("c",{"50","50","50"})
		typedef std::tuple< std::string, std::vector< std::string > > TextModifier;
		TextModifier parseModifier( const std::string& modifier );

		class RichText : public sf::Drawable, public sf::Transformable, private res::FontLoader<>
		{
		public:
			RichText();

			void clear();

			void changeColor( const sf::Color& color ) { m_curColor = color; }
			void changeStyle( int style ) { m_curStyle = style; }

			void loadFont( const std::string& font );

			const sf::Color& getCurColor() const { return m_curColor; }
			const sf::Color& getDefColor() const { return m_defaultColor; }

			sf::Text::Style getCurStyle() const { return (sf::Text::Style) m_curStyle; }
			const std::string& getString() const { return m_string; }

			void setCharacterSize( unsigned size );
			void setDefaultColor( const sf::Color& color );
			void setFieldWidth( float width );

			void setVisible( unsigned index ) { m_visibleIndex = index; }

		public:
			RichText& operator<<( const std::string& str );

			template< typename T >
			RichText& operator<<( const T& t )
			{
				std::ostringstream ss;
				ss << t;
				return *this << ss.str();
			}

		private:
			void insertString( const std::string& str );

			void update();
			void draw( sf::RenderTarget&, sf::RenderStates ) const;

		private:
			sf::Color m_curColor, m_defaultColor;
			int m_curStyle;

			float m_maxWidth;
			unsigned m_fontSize, m_numLines;

			std::string m_string;
			std::vector< sf::Text > m_words;
			unsigned m_visibleIndex;
		};
	}
}