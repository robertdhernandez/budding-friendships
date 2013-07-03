#include "mlpbf/utility/rich_text.h"
#include "mlpbf/exception.h"

#include <cctype>
#include <SFML/Graphics/RenderTarget.hpp>

namespace bf
{
namespace util
{

/***************************************************************************/

typedef std::vector< std::string > Arguments;

void textBold( RichText& text, const Arguments& args )
{
	text.changeStyle( text.getCurStyle() ^ sf::Text::Bold );
}

void textColor( RichText& text, const Arguments& args )
{
	unsigned size = args.size();
	if ( size == 0 )
		text.changeColor( text.getDefColor() );
	else if ( size == 3 )
		text.changeColor( sf::Color( std::stoi( args[ 0 ] ), std::stoi( args[ 1 ] ), std::stoi( args[ 2 ] ) ) );
	else
		throw Exception( "Color modifier must have zero or exactly three arguments" );
}

void textItalic( RichText& text, const Arguments& args )
{
	text.changeStyle( text.getCurStyle() ^ sf::Text::Italic );
}

void textUnderline( RichText& text, const Arguments& args )
{
	text.changeStyle( text.getCurStyle() ^ sf::Text::Underlined );
}

// cmd is everything inside two braces, braces not included
// Available modifiers:
//	{b}			toggles text bolding
//	{c}			resets color to default color
//	{c:x,y,z}	changes the current color
//	{i}			toggles text italicizing
//	{u}			toggles text underlining
bool modifyText( const std::string& cmd, RichText& text )
{
	TextModifier mod = parseModifier( cmd );

	const std::string& command = std::get< 0 >( mod );
	const std::vector< std::string >& args = std::get< 1 >( mod );

	bool success = false;

	// Execute a command
	if ( ( success = ( command == "b" ) ) )
		textBold( text, args );
	else if ( ( success = ( command == "c" ) ) )
		textColor( text, args );
	else if ( ( success = ( command == "i" ) ) )
		textItalic( text, args );
	else if ( ( success = ( command == "u" ) ) )
		textUnderline( text, args );

	return success;
}

/***************************************************************************/

inline void fillArguments( Arguments& args, const std::string& str )
{
	std::string::size_type posA = 0U;
	while ( posA != std::string::npos )
	{
		auto posB = str.find( ',', posA );
		if ( posB != std::string::npos )
		{
			args.push_back( str.substr( posA, posB - posA ) );
			posA = str.find_first_not_of( ' ', posB + 1 );
		}
		else
		{
			args.push_back( str.substr( posA ) );
			posA = posB;
		}
	}
}

TextModifier parseModifier( const std::string& str )
{
	TextModifier mod;
	std::string& command = std::get< 0 >( mod );

	// Get the string for the command
	auto colonPos = str.find( ':' );
	command = str.substr( 0U, colonPos );
	std::transform( command.begin(), command.end(), command.begin(), ::tolower );

	// Fill the arguments (if colon exists)
	if ( colonPos != std::string::npos )
		fillArguments( std::get< 1 >( mod ), str.substr( colonPos + 1 ) );

	return mod;
}

/***************************************************************************/

RichText::RichText() :
	m_curColor( sf::Color::Black ),
	m_defaultColor( sf::Color::Black ),
	m_curStyle( sf::Text::Regular ),
	m_maxWidth( 0.0f ),
	m_fontSize( 30U ),
	m_numLines( 1U ),
	m_visibleIndex( 0U )
{
}

void RichText::clear()
{
	m_curColor = m_defaultColor;
	m_curStyle = sf::Text::Regular;
	m_numLines = 1U;
	m_visibleIndex = 0U;
	m_words.clear();
	m_string.clear();
}

void RichText::loadFont( const std::string& font )
{
	res::FontLoader<>::loadFont( font );
	for ( auto it = m_words.begin(); it != m_words.end(); ++it )
		it->setFont( getFont() );
	update();
}

void RichText::setCharacterSize( unsigned size )
{
	m_fontSize = size;
	for ( auto it = m_words.begin(); it != m_words.end(); ++it )
		it->setCharacterSize( size );
	update();
}

void RichText::setDefaultColor( const sf::Color& color )
{
	if ( m_curColor == m_defaultColor )
		m_curColor = color;

	for ( auto it = m_words.begin(); it != m_words.end(); ++it )
		if ( it->getColor() == m_defaultColor )
			it->setColor( color );

	m_defaultColor = color;
}

void RichText::setFieldWidth( float width )
{
	m_maxWidth = std::max( 0.0f, width );
	update();
}

RichText& RichText::operator<<( const std::string& str )
{
	std::string::size_type posA = 0U;

	// Search until the first bracket
	while ( posA != std::string::npos )
	{
		auto posB = str.find( '{', posA );
		if ( posB != std::string::npos )
		{
			// Insert everything before the bracket
			insertString( str.substr( posA, posB - posA ) );

			// Find the closing bracket
			auto posC = str.find( '}', posB );
			if ( posC != std::string::npos )
			{
				// Move the posB inside the bracket
				posB++;

				// Modify the RichText accordingly
				if ( modifyText( str.substr( posB, posC - posB ), *this ) == false )
					insertString( str.substr( posB - 1, posC - posB + 2 ) ); // No valid command, insert everything including the braces

				// Set posA to the point after the closing bracket
				posA = posC + 1;
			}
			else
			{
				insertString( str.substr( posB ) );
				posA = posC; // posA = std::string::npos -- breaks loop
			}
		}
		else
		{
			// Insert the remainder of the text
			insertString( str.substr( posA ) );
			posA = posB; // posA = std::string::npos -- breaks loop
		}
	}

	return *this;
}

void RichText::insertString( const std::string& string )
{
	// Get the starting position, either the last word or (0,0)
	sf::Vector2f pos( 0.0f, 0.0f );
	if ( !m_words.empty() )
	{
		const sf::Text& last = m_words.back();
		pos = last.findCharacterPos( last.getString().getSize() );
	}

	bool getWord = true;
	std::string::size_type posA = 0, posB = std::string::npos;
	std::string word;

	// Loop through each word in the string, adding them individually
	while ( posA != std::string::npos )
	{
		// Search for the next position, be it either: a space or non-space
		posB = getWord ? string.find_first_of( ' ', posA ) : string.find_first_not_of( ' ', posA );

		// Get the substring
		word = string.substr( posA, posB != std::string::npos ? posB - posA : posB );

		//TODO: check if they contain special characters and act appropriately
		std::string::size_type newLinePos = word.find( '\n' );
		bool newLine = newLinePos != std::string::npos;

		if ( newLine )
		{
			insertString( word.substr( 0, newLinePos ) );
			word.erase( 0U, newLinePos + 1 );
		}

		m_string.append( word );

		// Add the word to the words vector and modify it accordingly
		m_words.push_back( sf::Text( word, getFont(), m_fontSize ) );
		sf::Text& input = m_words.back();

		input.setColor( m_curColor );
		input.setStyle( m_curStyle );
		input.setPosition( pos );

		// Check if the current word exceeds the limit
		if ( m_maxWidth != 0.0f || newLine )
		{
			pos = input.findCharacterPos( input.getString().getSize() );
			if ( newLine || ( m_maxWidth <= pos.x && getWord ) )
			{
				input.setPosition( sf::Vector2f( 0.0f, pos.y + getFont().getLineSpacing( m_fontSize ) ) );
				pos = input.findCharacterPos( input.getString().getSize() );
				m_numLines++;
			}
		}

		posA = posB;
		getWord = !getWord;
	}
}

void RichText::update()
{
	//TODO
}

void RichText::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	states.transform *= getTransform();

	unsigned i = 0U;

	for ( auto it = m_words.begin(); it != m_words.end(); ++it )
	{
		i += it->getString().getSize();
		if ( i <= m_visibleIndex ) 
			target.draw( *it, states );
		else
		{
			sf::Text copy = *it;
			std::string string = copy.getString();
			unsigned length = i - m_visibleIndex;
			copy.setString( string.substr( 0U, string.size() - length ) );

			target.draw( copy, states );
			break;
		}
	}
}

/***************************************************************************/

} // namespace util

} // namespace bf
