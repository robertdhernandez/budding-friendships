#include "mlpbf/console.h"
#include "mlpbf/console/function.h"

#include "mlpbf/global.h"
#include "mlpbf/exception.h"
#include "mlpbf/resource.h"

#include <algorithm>
#include <functional>
#include <iterator>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

namespace bf
{

static const float CONSOLE_OFFSET = 0.0f;
static const unsigned FONT_SIZE = 16U;
static const unsigned MAX_LINES = 250U;

sf::Color hexToColor( int color )
{
	return sf::Color( color >> 16, ( color >> 8 ) & 0xFF, color & 0xFF );
}

std::vector< std::string > parseArguments( const std::string& str )
{
	std::vector< std::string > args;
	if ( str.empty() ) return args;

	auto startWord	= str.find_first_not_of( ' ', 0 );
	auto endWord	= str.find_first_of( ' ', startWord );

	while ( startWord != std::string::npos || endWord != std::string::npos )
	{
		if ( str[ startWord ] == '\"' )
		{
			auto endQuote = str.find( '\"', startWord + 1 );
			if ( endQuote == std::string::npos )
				throw Exception( "argument(s) missing closing quote" );

			args.push_back( str.substr( startWord + 1, endQuote - startWord - 1 ) );

			startWord = str.find_first_not_of( ' ', endQuote + 1 );
			endWord = str.find_first_of( ' ', startWord );
		}
		else
		{
			args.push_back( str.substr( startWord, endWord - startWord ) );
			startWord = str.find_first_not_of( ' ', endWord );
			endWord = str.find_first_of( ' ', startWord );
		}
	}

	return args;
}

/***************************************************************************/

class UnknownCommandException : public Exception 
{ 
public: 
	UnknownCommandException() throw()
	{ 
		*this << "Unknown command"; 
	} 
};

class TooFewArgumentsException : public Exception
{
public:
	TooFewArgumentsException( const std::string& name, unsigned minArgs ) throw()
	{
		*this << "Requires at least " << minArgs << " arguments\nSee \"help " << name << "\" for more details";
	}
};

/***************************************************************************/

Console& Console::singleton()
{
	static Console console;
	return console;
}

Console::Console() :
	m_active( false ),
	m_index( 0 ),
	m_font( res::loadFont( "data/fonts/console.ttf" ) ),
	m_bufferOffset( 0 ),
	m_bufferColor( DEFAULT_COLOR )
{
	clearHistory();
	clearCommands();
}

/***************************************************************************/

void Console::addCommand( con::Command& cmd )
{
	m_cmds.push_back( &cmd );
	std::sort( m_cmds.begin(), m_cmds.end(), []( const con::Command* a, const con::Command* b ) { return *a < *b; } );
}

void Console::clearCommands()
{
	m_cmds.clear();
	con::defaultCommands( *this );
}

void Console::clearHistory()
{
	m_history.clear();
	m_bufferOffset = 0;
	pushLine( "Budding Friendships Console", INFO_COLOR );
}

void Console::pushBuffer()
{
	pushLine( m_buffer.str(), m_bufferColor );
	m_buffer.str( "" );
	m_bufferColor = DEFAULT_COLOR;
}

void Console::pushLine( const std::string& str, unsigned color )
{
	sf::Text text( "", *m_font, FONT_SIZE );
	std::string input;

	int insertCount = 0;
	for ( auto it = str.begin(); it != str.end(); ++it )
	{
		input.push_back( *it );

		text.setString( input );
		sf::FloatRect rect = text.getLocalBounds();
		if ( rect.width >= SCREEN_WIDTH )
		{
			std::string buffer;
			std::string::iterator ij = input.end() - 1;

			while ( *ij != ' ' )
			{
				buffer.push_back( *ij );
				input.erase( ij );
				ij--;
			}

			m_history.push_back( std::make_pair( input, color ) );

			input.clear();
			input.insert( input.begin(), buffer.rbegin(), buffer.rend() );
			insertCount++;
		}
		else if ( *it == '\n' )
		{
			m_history.push_back( std::make_pair( input, color ) );
			input.clear();
			insertCount++;
		}
	}

	if ( !insertCount ) 
		m_history.push_back( std::make_pair( str, color ) );
	else
		m_history.push_back( std::make_pair( input, color ) );

	while ( m_history.size() > MAX_LINES )
		m_history.erase( m_history.begin() );
}

void Console::execute( const std::string& line )
{
	std::string::size_type pos = line.find( ' ' );
	std::string cmd = std::string( line.begin(), pos != std::string::npos ? line.begin() + pos : line.end() );

	auto find = std::find_if( m_cmds.begin(), m_cmds.end(), std::bind( &con::Command::operator==, std::placeholders::_1, cmd ) );

	try
	{
		if ( find == m_cmds.end() )
			throw UnknownCommandException();
		(**find)( *this, parseArguments( pos != std::string::npos ? std::string( line.begin() + pos + 1, line.end() ) : "" ) );
	}
	catch ( std::exception& err )
	{
		*this << con::setcerr << err.what() << con::endl;
	}
}

/***************************************************************************/

void Console::onKeyPressed( const sf::Event::KeyEvent& ev )
{
	if ( ev.code == sf::Keyboard::Tilde )
		m_active = !m_active;

	if ( m_active )
	{
		switch ( ev.code )
		{
		case sf::Keyboard::BackSpace: 
			if ( !m_input.empty() && m_index > 0 ) 
			{
				m_input.erase( m_input.begin() + m_index - 1 ); 
				m_index--;
			}
		break;

		case sf::Keyboard::Delete:
			if ( !m_input.empty() && m_index != m_input.size() )
				m_input.erase( m_input.begin() + m_index );
		break;
	
		case sf::Keyboard::Return:
			if ( !m_input.empty() )
			{
				pushLine( m_input );
				execute( m_input );
				m_index = 0;
				m_input.clear();
			}
		break;

		case sf::Keyboard::PageUp:
			m_bufferOffset = std::min( (int) m_history.size() - 1, m_bufferOffset + 1 );
		break;

		case sf::Keyboard::PageDown:
			m_bufferOffset = std::max( 0, m_bufferOffset - 1 );
		break;

		case sf::Keyboard::Home:
			m_bufferOffset = m_history.size() - 1;
		break;

		case sf::Keyboard::End:
			m_bufferOffset = 0;
		break;

		case sf::Keyboard::Left:
			m_index = std::max( 0, m_index - 1 );
		break;

		case sf::Keyboard::Right:
			m_index = std::min( (int) m_input.size(), m_index + 1 );
		break;

		case Console::KEY:
			m_active = false;
		break;
		}
	}
}

void Console::onKeyReleased( const sf::Event::KeyEvent& ev )
{
}

void Console::onTextEntered( const sf::Event::TextEvent& ev )
{
	if ( m_active && ev.unicode >= ' ' && ev.unicode != '`' )
	{
		m_input.insert( m_input.begin() + m_index, ev.unicode );
		m_index++;
	}
}

/***************************************************************************/

void Console::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	if ( !m_active ) return;

	// Background 
	sf::RectangleShape bg( sf::Vector2f( (float) SCREEN_WIDTH, (float) SCREEN_HEIGHT ) );
	bg.setFillColor( sf::Color( 150, 150, 150, 150 ) );
	target.draw( bg );

	sf::Text text( "", *m_font, FONT_SIZE );
	int height = text.getFont()->getLineSpacing( FONT_SIZE );
	float yPos = SCREEN_HEIGHT - CONSOLE_OFFSET - height;

	// Current input
	{
		// Before index
		text.setString( ">" + std::string( m_input.begin(), m_input.begin() + m_index ) );
		text.setPosition( 0.0f, yPos );
		text.setColor( hexToColor( DEFAULT_COLOR ) );
		target.draw( text );
		
		if ( m_index < (int) m_input.size() )
		{
			float xPos = text.findCharacterPos( m_index + 1 ).x;

			// Index char
			text.setString( m_input.at( m_index ) );
			text.setPosition( xPos, yPos );
			text.setStyle( sf::Text::Underlined );
			target.draw( text );

			// After index
			xPos += text.getLocalBounds().width;
			text.setString( std::string( m_input.begin() + m_index + 1, m_input.end() ) );
			text.setPosition( xPos, yPos );
			text.setStyle( sf::Text::Regular );
			target.draw( text );
		}
		else
		{
			text.setPosition( text.findCharacterPos( m_index + 1 ).x, yPos );
			text.setString( " " );
			text.setStyle( sf::Text::Underlined );
			target.draw( text );

			// Reset style
			text.setStyle( sf::Text::Regular );
		}
	}

	// History
	for ( auto it = m_history.rbegin() + m_bufferOffset; it != m_history.rend() && yPos >= 0.0f; ++it )
	{
		text.setPosition( 0.0f, yPos -= height );
		text.setString( it->first );
		text.setColor( hexToColor( it->second ) );
		target.draw( text );
	}
}

/***************************************************************************/

void con::Command::operator()( Console& c, const std::vector< std::string >& args ) const
{
	unsigned argReq = minArgs();
	auto size = args.size();

	if ( size < argReq )
		throw TooFewArgumentsException( name(), argReq );
	else
		execute( c, args );
}

/***************************************************************************/

void con::endl( Console& c )
{
	c.pushBuffer();
}

void con::setcdef( Console& c )
{
	c.setBufferColor( Console::DEFAULT_COLOR );
}

void con::setcerr( Console& c )
{
	c.setBufferColor( Console::ERROR_COLOR );
}

void con::setcinfo( Console& c )
{
	c.setBufferColor( Console::INFO_COLOR );
}

/***************************************************************************/

} // namespace bf
