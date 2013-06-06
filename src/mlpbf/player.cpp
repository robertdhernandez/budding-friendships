#include "mlpbf/player.h"
#include "mlpbf/direction.h"

namespace bf
{

static const float USE_OFFSET = 15.0f;

/***************************************************************************/

Player& Player::singleton()
{
	static Player p;
	return p;
}

Player::Player() :
	Character( "player_a" ),
	m_invLevel( 0U ),
	m_inv( 0 )
{
	setInventoryLevel( 0U );
}

sf::Vector2f Player::getUsePosition() const
{
	const sf::FloatRect& rect = getBounds();
	const sf::Vector2f& pos = getPosition();
	switch ( getDirection() )
	{
	case Up:		return sf::Vector2f( pos.x, pos.y - ( rect.height / 2.0f ) - USE_OFFSET );
	case Down:	return sf::Vector2f( pos.x, pos.y + ( rect.height / 2.0f ) + USE_OFFSET );
	case Left:	return sf::Vector2f( pos.x - ( rect.width / 2.0f ) - USE_OFFSET, pos.y );
	case Right:	return sf::Vector2f( pos.x + ( rect.width / 2.0f ) + USE_OFFSET, pos.y );
	}
	return pos;
}

void Player::setInventoryLevel( sf::Uint8 level )
{
	m_invLevel = std::min( level, ( sf::Uint8 ) 5U );
	switch ( m_invLevel )
	{
	case 0: m_inv.setSize( 3 );  break;
	case 1: m_inv.setSize( 6 );  break;
	case 2: m_inv.setSize( 9 );  break;
	case 3: m_inv.setSize( 12 ); break;
	case 4: m_inv.setSize( 16 ); break;
	case 5: m_inv.setSize( 20 ); break;
	}
}

/***************************************************************************/

} // namespace bf
