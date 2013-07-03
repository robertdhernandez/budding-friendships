#include "mlpbf/global.h"
#include "mlpbf/console.h"
#include "mlpbf/exception.h"

#include "mlpbf/state/base.h"
#include "mlpbf/resource.h"
#include "mlpbf/ui/window.h"
#include "mlpbf/utility/rich_text.h"
#include "mlpbf/utility/timer.h"

#include <memory>
#include <queue>

#include <SFML/Graphics/RenderTarget.hpp>

namespace bf
{

static const sf::Vector2f	DIALOGUE_SPEAKER_OFFSET		= sf::Vector2f( 15.0f, 5.0f );
static const unsigned		DIALOGUE_SPEAKER_SIZE		= 30U;
static const sf::Color		DIALOGUE_SPEAKER_COLOR		= sf::Color( 85U, 68U, 0U );
static const std::string	DIALOGUE_SPEAKER_FONT			= "data/fonts/CelestiaMediumRedux1.5.ttf";

static const sf::Vector2f	DIALOGUE_MESSAGE_OFFSET		= sf::Vector2f( 30.0f, 45.0f );
static const unsigned		DIALOGUE_MESSAGE_SIZE		= 16U;
static const sf::Color		DIALOGUE_MESSAGE_COLOR		= sf::Color( 85U, 68U, 0U );
static const std::string	DIALOGUE_MESSAGE_FONT			= "data/fonts/mvboli.ttf";
static const float			DIALOGUE_MESSAGE_WIDTH		= 548.0f;
static const unsigned		DIALOGUE_MESSAGE_NUMLINES	= 3U;

static const float			DIALOGUE_OFFSET			= SCREEN_HEIGHT * ( 5.0f / 6.0f );
static const sf::Time		DIALOGUE_MOVE_TIME			= sf::milliseconds( 375 );

static const sf::Time		DIALOGUE_UPDATE_TIME		= sf::milliseconds( 30 );

/***************************************************************************/

class Dialogue;

// FORWARD DECLARATION
class RuntimeModifier
{
public:
	friend class Message;

	RuntimeModifier() {}
	virtual ~RuntimeModifier() {}

	// Parse arguments
	virtual void parse( const std::vector< std::string >& args ) = 0;

	// Returns true when finished
	virtual bool execute( Dialogue& parent ) = 0;

protected:
	RuntimeModifier( const std::string& line ) : m_line( line ) {}

private:
	std::string m_line;
};

class TextModifier : public RuntimeModifier
{
public:
	TextModifier( const std::string& line ) : RuntimeModifier( line ) {}

private:
	void parse( const std::vector< std::string >& args ) {}
	bool execute( Dialogue& ) { return true; }
};

std::unique_ptr< RuntimeModifier > getRuntimeModifier( const std::string& str );

/***************************************************************************/

class Speaker : public ui::Base, res::FontLoader<>
{
public:
	Speaker( const std::string& speaker ) :
		m_speaker( speaker )
	{
		loadFont( DIALOGUE_SPEAKER_FONT );
		setPosition( DIALOGUE_SPEAKER_OFFSET );
	}

	void setString( const std::string& speaker )
	{
		m_speaker = speaker;
	}

private:
	void onUpdate()
	{
	}

	void draw( sf::RenderTarget& target, sf::RenderStates states ) const
	{
		states.transform *= getTransform();

		sf::Text speaker( m_speaker, getFont(), DIALOGUE_SPEAKER_SIZE );
		speaker.setColor( DIALOGUE_SPEAKER_COLOR );

		target.draw( speaker, states );
	}

private:
	std::string m_speaker;
};

/***************************************************************************/

class Message : public ui::Base
{
public:
	Message( Dialogue& parent, const std::string& message ) :
		m_parent( parent ),
		m_index( 0U ),
		m_updateTime( DIALOGUE_UPDATE_TIME ),
		m_updateMod( 1.0f )
	{
		m_timer.setTarget( m_updateTime * m_updateMod );
		m_timer.setState( true );

		setPosition( DIALOGUE_MESSAGE_OFFSET );

		m_text.loadFont( DIALOGUE_MESSAGE_FONT );
		m_text.setFieldWidth( DIALOGUE_MESSAGE_WIDTH );
		m_text.setCharacterSize( DIALOGUE_MESSAGE_SIZE );
		m_text.setDefaultColor( DIALOGUE_MESSAGE_COLOR );

		appendString( message );
	}

	void appendString( const std::string& message )
	{
		// Parse the text for any run-time commands
		std::string::size_type leftBrace = message.find( '{' ), rightBrace, last = 0U;

		if ( leftBrace != std::string::npos )
		{
			while ( leftBrace != std::string::npos )
			{
				rightBrace = message.find( '}', leftBrace );
				if ( rightBrace != std::string::npos )
				{
					auto insert = getRuntimeModifier( message.substr( leftBrace + 1, rightBrace - leftBrace - 1 ) );
					if ( insert ) // Valid runtime command
					{
						insert->m_line = message.substr( last, leftBrace - last );
						m_modifiers.push( std::move( insert ) );
						last = rightBrace + 1; // Move the last index to after the closing bracket
					}

					// Find the next left bracket
					leftBrace = message.find( '{', rightBrace );

					// If no more brackets, insert the rest of the string as a TextModifier
					if ( leftBrace == std::string::npos )
						m_modifiers.push( std::unique_ptr< RuntimeModifier >( new TextModifier( message.substr( last ) ) ) );
				}
				else // Insert the rest of the string as a TextModifier
				{
					m_modifiers.push( std::unique_ptr< RuntimeModifier >( new TextModifier( message.substr( leftBrace ) ) ) );
					leftBrace = rightBrace; // leftBrace = std::string::npos
				}
			}
			m_text << m_modifiers.front()->m_line;
		}
		else
			m_text << message;
	}

	void clear()
	{
		m_text.clear();
		m_index = 0U;
		m_timer.restart();
	}

	void setState( bool state )
	{
		m_timer.setState( state );
	}

	bool isPaused() const
	{
		return !m_timer.getState() && !isFinished();
	}

	bool isFinished() const
	{
		return m_modifiers.empty() && m_index == m_text.getString().size();
	}

	void setModifier( float mod )
	{
		m_updateMod = std::max( 0.0f, mod );
		m_timer.setTarget( m_updateTime * m_updateMod );
	}

	void setUpdateTime( const sf::Time& time )
	{
		m_updateTime = time;
		m_timer.setTarget( m_updateTime * m_updateMod );
	}

private:
	void onUpdate()
	{
		// Reached end of text and modifiers available
		if ( m_index == m_text.getString().size() && !m_modifiers.empty() )
		{
			if ( m_modifiers.front()->execute( m_parent ) )
			{
				m_modifiers.pop();
				if ( !m_modifiers.empty() )
					m_text << m_modifiers.front()->m_line;
			}
		}
		else if ( m_timer.getState() && !isFinished() && m_timer )
		{
			const std::string& message = m_text.getString();

			m_index = message.find_first_not_of( ' ', m_index + 1 );
			if ( m_index == std::string::npos ) 
				m_index = message.size();

			m_timer.restart();
			m_text.setVisible( m_index );
		}
	}

	void draw( sf::RenderTarget& target, sf::RenderStates states ) const
	{
		states.transform *= getTransform();
		target.draw( m_text, states );
	}

private:
	Dialogue& m_parent;
	std::size_t m_index;

	sf::Time m_updateTime;
	float m_updateMod;

	util::Timer m_timer;
	util::RichText m_text;

	std::queue< std::unique_ptr< RuntimeModifier > > m_modifiers;
};

/***************************************************************************/

class Dialogue : public ui::Window
{
public:
	Dialogue( const std::string& speaker, const std::string& message ) :
		m_updated( false ),
		m_fast( false )
	{
		loadTexture( "data/ui/dialogue.png" );

		const sf::Vector2u size = getTexture().getSize();
		setOrigin( size.x / 2.0f, size.y / 2.0f );
		setPosition( SCREEN_WIDTH / 2.0f, (float) SCREEN_HEIGHT + size.y );

		// Note: deallocation is handled by ui::Window
		// These variables are just for self reference
		m_speaker = new Speaker( speaker );
		m_message = new Message( *this, message );

		addChild( m_speaker );
		addChild( m_message );
	}

	Dialogue& addLine( const std::string& speaker, const std::string& message )
	{
		m_queue.push( make_pair( speaker, message ) );
		return *this;
	}

	Speaker& getSpeaker()
	{
		return *m_speaker;
	}

	Message& getMessage()
	{
		return *m_message;
	}

private:
	void onOpen()
	{
		move( sf::Vector2f( SCREEN_WIDTH / 2.0f, DIALOGUE_OFFSET ), DIALOGUE_MOVE_TIME );
	}

	bool opened() const
	{
		return !isMoving();
	}

	void onClose()
	{
		move( sf::Vector2f( SCREEN_WIDTH / 2.0f, (float) SCREEN_HEIGHT + getTexture().getSize().y ), DIALOGUE_MOVE_TIME );
	}

	bool closed() const
	{
		return !isMoving();
	}

	void onWindowUpdate()
	{
		Message& message = getMessage();

		if ( m_updated && !message.isFinished() )
		{
			message.setModifier( m_fast ? 0.25f : 1.0f );
			m_updated = false;
		}
	}

	void onKeyPressed( const sf::Event::KeyEvent& ev )
	{
		Speaker& speaker = getSpeaker();
		Message& message = getMessage();

		switch ( ev.code )
		{
		case sf::Keyboard::Z:
			m_updated = true;
			if ( message.isPaused() )
				message.setState( true );
			else if ( message.isFinished() && !m_fast && !m_queue.empty() )
			{
				const auto& pair = m_queue.front();
				message.clear();
				message.appendString( pair.second );
				speaker.setString( pair.first );
				m_queue.pop();
			}
			else if ( !message.isFinished() )
				m_fast = true;
		break;

		case Console::KEY:
			Console::singleton().state( true );
		break;
		
		default: break;
		}
	}

	void onKeyReleased( const sf::Event::KeyEvent& ev )
	{
		Message& message = getMessage();

		switch ( ev.code )
		{
		case sf::Keyboard::Z:
			if ( !message.isPaused() && m_fast )
			{
				m_fast = false;
				m_updated = true;
			}
			else if ( message.isFinished() )
				close();
		break;
		
		default: break;
		}
	}

private:
	bool m_updated, m_fast;
	Speaker* m_speaker;
	Message* m_message;
	std::queue< std::pair< std::string, std::string > > m_queue;
};

/***************************************************************************/

class SpeedModifier : public RuntimeModifier
{
	void parse( const std::vector< std::string >& args )
	{
		m_speed = DIALOGUE_UPDATE_TIME; // default
		if ( args.size() >= 1 )
			m_speed = sf::milliseconds( std::stoi( args[ 0 ] ) );
	}

	bool execute( Dialogue& parent )
	{
		parent.getMessage().setUpdateTime( m_speed );
		return true;
	}

	sf::Time m_speed;
};

class PauseModifier : public RuntimeModifier 
{
	void parse( const std::vector< std::string >& args )
	{
		m_init = false;
		m_pause = true;
		if ( args.size() >= 1 )
		{
			m_timer.setTarget( sf::milliseconds( std::stoi( args[ 0 ] ) ) );
			m_pause = false;
		}
	}

	bool execute( Dialogue& parent )
	{
		if ( !m_init )
		{
			if ( m_pause )
				parent.getMessage().setState( false );
			else
				m_timer.setState( true );

			m_init = true;
			return false;
		}

		return m_pause ? !parent.getMessage().isPaused() : m_timer.finished();
	}

	bool m_pause, m_init;
	util::Timer m_timer;
};

std::unique_ptr< RuntimeModifier > getRuntimeModifier( const std::string& str )
{
	util::TextModifier mod = util::parseModifier( str );
	const std::string& cmd = std::get< 0 >( mod );
	const std::vector< std::string >& args = std::get< 1 >( mod );

	std::unique_ptr< RuntimeModifier > val;

	if ( cmd == "p" )
		val.reset( new PauseModifier() );
	else if ( cmd == "s" )
		val.reset( new SpeedModifier() );
	
	if ( val ) 
		val->parse( args );

	return val;
}

/***************************************************************************/

void showText( const std::string& message, const std::string& speaker )
{
	if ( ui::Window::getGlobal() )
	{
		Dialogue* d = dynamic_cast< Dialogue* >( ui::Window::getGlobal() );
		if ( d == nullptr )
			throw Exception( "window in use" );
		d->addLine( speaker, message );
	}
	else
	{
		ui::Window::setGlobal( new Dialogue( speaker, message ) );
		state::global().setKeyListener( *ui::Window::getGlobal() );
	}
}

/***************************************************************************/

} // namespace bf
