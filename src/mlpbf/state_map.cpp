#include "mlpbf/state/map.h"

#include "mlpbf/global.h"
#include "mlpbf/console.h"
#include "mlpbf/direction.h"
#include "mlpbf/player.h"
#include "mlpbf/map.h"

#include "mlpbf/time.h"
#include "mlpbf/ui/window.h"

#include <algorithm>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace bf
{

static const sf::Keyboard::Key KEY_MOVE_UP		= sf::Keyboard::Up;
static const sf::Keyboard::Key KEY_MOVE_DOWN		= sf::Keyboard::Down;
static const sf::Keyboard::Key KEY_MOVE_LEFT		= sf::Keyboard::Left;
static const sf::Keyboard::Key KEY_MOVE_RIGHT	= sf::Keyboard::Right;

static const sf::Keyboard::Key KEY_MOVE_FAST		= sf::Keyboard::LShift;
static const sf::Keyboard::Key KEY_MOVE_SLOW		= sf::Keyboard::LControl;

static const sf::Keyboard::Key KEY_PRIMARY		= sf::Keyboard::Z;
static const sf::Keyboard::Key KEY_SECONDARY		= sf::Keyboard::X;

static const sf::Keyboard::Key KEY_INVENTORY		= sf::Keyboard::I;

//DEBUG
static Character * rarity;
static std::size_t inventoryIndex;

/***************************************************************************/

state::Map::Map() :
	m_viewer( bf::Map::global() ),
	m_clock( Time::singleton().getDate(), Time::singleton().getHour() ),
	m_dir( Player::singleton().getDirection() ),
	m_moving( false ),
	m_speedModifier( Default ),
	m_updateSprite( true )
{
	setKeyListener( *this );

	m_viewer.addCharacter( Player::singleton() );
	
	rarity = new Character( "rarity" );
	m_viewer.addCharacter( *rarity );

	rarity->reposition( sf::Vector2i( 14, 14 ), "path_a" )
		  .repeatBegin( 2U )
			.repeatBegin( 3U )
			  .move( Right, Walk, 4U )
			  .move( Left, Walk, 4U )
			.repeatEnd()
			.face( Up )
			.wait( sf::milliseconds( 2500 ) )
			.repeatBegin( 3U )
			  .move( Down, Walk, 4U )
			  .move( Up, Walk, 4U )
			.repeatEnd()
			.face( Left )
			.wait( sf::milliseconds( 2500 ) )
		  .repeatEnd();

		  //.wait( sf::milliseconds( 2500 ) )
		  //.face( Left )
		  //.wait( sf::milliseconds( 2500 ) )
		  //.face( Up )
		  //.wait( sf::milliseconds( 2500 ) )
		  //.face( Down )
		  //.wait( sf::milliseconds( 2500 ) )
		  //.move( Right, Walk, 60 )
		  //.wait( sf::milliseconds( 2500 ) )
		  //.move( Down, Walk, 60 )
		  //.wait( sf::milliseconds( 2500 ) )
		  //.move( Left, Walk, 60 )
		  //.wait( sf::milliseconds( 2500 ) )
		  //.move( Up, Walk, 60 )
		  //.face( Right )
		  //.reposition( sf::Vector2i( 16, 16 ), "farm" );
	
	inventoryIndex = 0U;

	item::Inventory& inventory = Player::singleton().getInventory();
	inventory.addItem( "turnip" );
	inventory.addItem( "cucumber" );
	inventory.addItem( "potato" );
	//inventory.addItem( "strawberry" );

	Time::singleton().setState( true );
}

void state::Map::onKeyPressed( const sf::Event::KeyEvent& ev )
{
	switch ( ev.code )
	{
	case KEY_MOVE_UP:
		if ( m_dir != Up || !m_moving )
		{
			m_dir = Up;
			m_moving = m_updateSprite = true;
		}
	break;

	case KEY_MOVE_DOWN:
		if ( m_dir != Down || !m_moving )
		{
			m_dir = Down;
			m_moving = m_updateSprite = true;
		}
	break;

	case KEY_MOVE_LEFT:
		if ( m_dir != Left || !m_moving )
		{
			m_dir = Left;
			m_moving = m_updateSprite = true;
		}
	break;

	case KEY_MOVE_RIGHT:
		if ( m_dir != Right || !m_moving )
		{
			m_dir = Right;
			m_moving = m_updateSprite = true;
		}
	break;

	case KEY_MOVE_FAST:
		if ( m_speedModifier == Default )
		{
			m_speedModifier = Fast;
			m_updateSprite = true;
		}
	break;

	case KEY_MOVE_SLOW:
		if ( m_speedModifier == Default )
		{
			m_speedModifier = Slow;
			m_updateSprite = true;
		}
	break;

	case KEY_PRIMARY:
		bf::Map::global().interact( bf::Player::singleton().getUsePosition() );
	break;

	case sf::Keyboard::Q:
		if ( ui::Window::getGlobal() )
			ui::Window::getGlobal()->close();
		else
		{
			showText( "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut." );
			showText( "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut." );
			showText( "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut." );
		}
	break;

	//case sf::Keyboard::Comma:
	//	inventoryIndex = std::max( inventoryIndex - 1, 0U );
	//break;

	//case sf::Keyboard::Period:
	//	inventoryIndex = std::min( inventoryIndex + 1, inventory.size() - 1 );
	//break;

	//case sf::Keyboard::Slash:
	//	{
	//		const ItemPtr& item = inventory[ inventoryIndex ];
	//		Console::singleton() << con::setcinfo;
	//		if ( item )
	//			Console::singleton() << "[" << item->getID() << "] " << item->getName() << ": " << item->getDesc() << con::endl;
	//		else
	//			Console::singleton() << "N/A" << con::endl;
	//	}
	//break;

	case Console::KEY:
		m_moving = false;
		m_updateSprite = true;
		Console::singleton().state( true );
	break;
	}
}

void state::Map::onKeyReleased( const sf::Event::KeyEvent& ev )
{
	switch ( ev.code )
	{
	case KEY_MOVE_UP:
		if ( m_dir == Up )
		{
			m_moving = false;
			m_updateSprite = true;
		}
	break;

	case KEY_MOVE_DOWN:
		if ( m_dir == Down )
		{
			m_moving = false;
			m_updateSprite = true;
		}
	break;

	case KEY_MOVE_LEFT:
		if ( m_dir == Left )
		{
			m_moving = false;
			m_updateSprite = true;
		}
	break;

	case KEY_MOVE_RIGHT:
		if ( m_dir == Right )
		{
			m_moving = false;
			m_updateSprite = true;
		}
	break;

	case KEY_MOVE_FAST:
		if ( m_speedModifier == Fast )
		{
			m_speedModifier = Default;
			m_updateSprite = true;
		}
	break;

	case KEY_MOVE_SLOW:
		if ( m_speedModifier == Slow )
		{
			m_speedModifier = Default;
			m_updateSprite = true;
		}
	break;

	case sf::Keyboard::A:
		showInventory();
	break;
	}
}

void state::Map::update( const sf::Time& time )
{	
	bf::Player& player = bf::Player::singleton();
	bf::Map& map = bf::Map::global();

	if ( Console::singleton().state() )
	{
		setKeyListener( Console::singleton() );
		setTextListener( Console::singleton() );
	}
	else if ( ui::Window::getGlobal() ) 
	{
		ui::Window& container = *ui::Window::getGlobal();

		if ( container.isClosing() )
		{
			setKeyListener( *this );
			removeTextListener();
		}
		else
		{
			setKeyListener( container );
			removeTextListener();
		}

		if ( m_clock.isVisible() )
		{
			m_clock.setVisible( false );
			m_moving = false;
			player.setMovement( Idle, m_dir );
		}

		if ( ui::Window::getGlobal()->canRemove() )
		{
			ui::Window::setGlobal( nullptr );
			m_clock.setVisible( true );
		}
		else
			ui::Window::getGlobal()->update();
	}
	else
	{
		setKeyListener( *this );
		removeTextListener();
	}

	// Update the player sprite if an input occured to change the sprite
	if ( !player.hasActions() && m_updateSprite )
	{
		MoveSpeed m = Idle;
		if ( m_moving )
			switch ( m_speedModifier )
			{
			case Default:	m = Trot;	break;
			case Fast:		m = Run;	break;
			case Slow:		m = Walk;	break;
			}

		player.setMovement( m, m_dir );
		m_updateSprite = false;
	}

	// Update time
	Time::singleton().update();

	// Move the player depending on the input
	player.update( time );

	//DEBUG: Update Rarity's path
	rarity->update( time );

	// Update the clock UI
	m_clock.update();

	// Change the map viewer refers to if the global map changed
	if ( player.getMapID() != m_viewer.map().getID() )
		m_viewer.map( bf::Map::global( player.getMapID() ) );

	// Center the map on the player
	m_viewer.center( player.getPosition() );

	// Update the current map
	map.update( time.asMilliseconds(), player.getPosition() );
}

void state::Map::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	target.draw( m_viewer, states );
	
	//TODO: check if exterior
	time::drawHourTint( target, Time::singleton().getHour() );

	//DEBUG
	if ( DEBUG_COLLISION )
	{
		const sf::FloatRect& rect = m_viewer.getViewArea();
		sf::CircleShape usePos;
		usePos.setFillColor( sf::Color::Red );
		usePos.setPosition( Player::singleton().getUsePosition() - sf::Vector2f( rect.left, rect.top ) );
		usePos.setRadius( 3.0f );
		usePos.setOrigin( 1.5f, 1.5f );
		target.draw( usePos, states );
	}

	target.draw( m_clock, states );

	if ( ui::Window::getGlobal() )
		target.draw( *ui::Window::getGlobal(), states );
}

/***************************************************************************/

} // namespace bf
