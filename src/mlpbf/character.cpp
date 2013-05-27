#include "mlpbf/character.h"

#include "mlpbf/global.h"
#include "mlpbf/direction.h"
#include "mlpbf/map.h"
#include "mlpbf/database/map.h"
#include "mlpbf/exception.h"

#include <sstream>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace bf
{

const std::string strDirection( Direction d )
{
	switch ( d )
	{
	case Up:	return "up";
	case Down:	return "down";
	case Left:	return "left";
	case Right:	return "right";
	}
	throw Exception( "strDirection recieved a bad Direction enum" );
}

const std::string strMoveSpeed( MoveSpeed m )
{
	switch ( m )
	{
	case Idle:	return "idle";
	case Walk:	return "walk";
	case Trot:	return "trot";
	case Run:	return "run";
	}
	throw Exception( "strMoveSpeed recieved a bad MoveSpeed enum" );
}

inline sf::Vector2f getMoveSpeed( MoveSpeed m, Direction d )
{
	float speed = 0.0f;

	switch ( m )
	{
	case Idle:	speed = 0.00f; break;
	case Walk:	speed = 0.50f; break;
	case Trot:	speed = 1.00f; break;
	case Run:	speed = 2.00f; break;
	}

	switch ( d )
	{
	case Up:	return sf::Vector2f( 0.0f, -speed );
	case Down:	return sf::Vector2f( 0.0f, speed );
	case Left:	return sf::Vector2f( -speed, 0.0f );
	case Right:	return sf::Vector2f( speed, 0.0f );
	}

	throw Exception( "getMoveSpeed could not generate a move speed" );
}

sf::Vector2u convert( const sf::Vector2f& pos )
{
	return sf::Vector2u( (int) pos.x / TILE_WIDTH, (int) pos.y / TILE_HEIGHT );
}

/***************************************************************************/

Character::Character( const std::string& spritesheet ) :
	m_mapID( 0U )
{
	m_sheet.load( spritesheet );
	setMovement( Idle, Down );
}

/***************************************************************************/

void Character::setMap( const std::string& map )
{
	setMap( map, m_pos );
}

void Character::setMap( const std::string& map, const sf::Vector2f& pos )
{
	m_mapID = db::Map::singleton()[ map ].getID();
	m_pos = pos;
}

void Character::update( const sf::Time& time )
{
	updateCharacter( *this );

	const Map& m = db::Map::singleton()[ m_mapID ];

	sf::Vector2f move = std::get< 1 >( m_move ) * ( time.asMilliseconds() / 10.0f );
	if ( move == sf::Vector2f( 0.0f, 0.0f ) ) return;

	// Move the character bounds
	sf::FloatRect bound = getBounds();

	// Check for collision depending on direction
	sf::Vector2f checkA, checkB;
	bool collision = false;
	
	//TODO: factor out code
	switch ( std::get< 0 >( m_move ) )
	{
	case Up: // Top-left and top-right
		checkA = sf::Vector2f( bound.left, bound.top + move.y );
		checkB = sf::Vector2f( bound.left + bound.width, checkA.y );

		while ( m.checkTileCollision( convert( checkA ) ) || m.checkObjectCollision( checkA ) ||
				m.checkTileCollision( convert( checkB ) ) || m.checkObjectCollision( checkB ) )
		{
			if ( !collision )
			{
				checkA.y = checkB.y = std::floor( checkB.y / TILE_HEIGHT ) * TILE_HEIGHT + 1.0f;
				collision = true;
			}
			else
				checkA.y = checkB.y += TILE_HEIGHT;
		}

		bound.top = checkA.y;
	break;

	case Down: // Bottom-left and bottom-right
		checkA = sf::Vector2f( bound.left, bound.top + bound.height + move.y );
		checkB = sf::Vector2f( bound.left + bound.width, checkA.y );

		while ( m.checkTileCollision( convert( checkA ) ) || m.checkObjectCollision( checkA ) ||
				m.checkTileCollision( convert( checkB ) ) || m.checkObjectCollision( checkB ) )
		{
			if ( !collision )
			{
				checkA.y = checkB.y = std::ceil( checkB.y / TILE_HEIGHT ) * TILE_HEIGHT - 1.0f;
				collision = true;
			}
			else
				checkA.y = checkB.y += -TILE_HEIGHT;
		}

		bound.top = checkA.y - bound.height;
	break;

	case Left: // Top-left and bottom-left
		checkA = sf::Vector2f( bound.left + move.x, bound.top ); 
		checkB = sf::Vector2f( checkA.x, bound.top + bound.height );

		while ( m.checkTileCollision( convert( checkA ) ) || m.checkObjectCollision( checkA ) ||
				m.checkTileCollision( convert( checkB ) ) || m.checkObjectCollision( checkB ) )
		{
			if ( !collision )
			{
				checkA.x = checkB.x = std::floor( checkB.x / TILE_WIDTH ) * TILE_WIDTH + 1.0f;
				collision = true;
			}
			else
				checkA.x = checkB.x += TILE_WIDTH;
		}

		bound.left = checkA.x;
	break;

	case Right:	// Top-right and bottom-right
		checkA = sf::Vector2f( bound.left + bound.width + move.x, bound.top );
		checkB = sf::Vector2f( checkA.x, bound.top + bound.height );

		while ( m.checkTileCollision( convert( checkA ) ) || m.checkObjectCollision( checkA ) ||
				m.checkTileCollision( convert( checkB ) ) || m.checkObjectCollision( checkB ) )
		{
			if ( !collision )
			{
				checkA.x = checkB.x = std::ceil( checkB.x / TILE_WIDTH ) * TILE_WIDTH - 1.0f;
				collision = true;
			}
			else
				checkA.x = checkB.x += -TILE_WIDTH;
		}

		bound.left = checkA.x - bound.width;
	break;
	}

	if ( collision )
		setMovement( Idle, std::get< 0 >( m_move ) );

	std::get< 2 >( m_move ) = collision;

	m_pos.x = bound.left + ( bound.width / 2.0f );
	m_pos.y = bound.top + ( bound.height / 2.0f );

	// Change the character's current map if they left the map bounds
	const Map& curMap = db::Map::singleton()[ m_mapID ], *nextMap = nullptr;
	if ( ( nextMap = curMap.getNeighbor( Up ) ) != nullptr && m_pos.y < 0.0f )
	{
		m_mapID = nextMap->getID();
		m_pos.x = m_pos.x + ( curMap.getNeighborOffset( Up ) * TILE_WIDTH );
		m_pos.y = nextMap->getHeight() * TILE_HEIGHT - m_pos.y;
	}
	else if ( ( nextMap = curMap.getNeighbor( Down ) ) != nullptr && curMap.getHeight() * TILE_HEIGHT <= m_pos.y )
	{
		m_mapID = nextMap->getID();
		m_pos.x = m_pos.x + ( curMap.getNeighborOffset( Down ) * TILE_HEIGHT );
		m_pos.y = m_pos.y - ( curMap.getHeight() * TILE_HEIGHT );
	}
	else if ( ( nextMap = curMap.getNeighbor( Left ) ) != nullptr && m_pos.x < 0.0f )
	{
		m_mapID = nextMap->getID();
		m_pos.x = nextMap->getWidth() * TILE_WIDTH - m_pos.x;
		m_pos.y = m_pos.y + ( curMap.getNeighborOffset( Left ) * TILE_HEIGHT );
	}
	else if ( ( nextMap = curMap.getNeighbor( Right ) ) != nullptr && curMap.getWidth() * TILE_WIDTH <= m_pos.x )
	{
		m_mapID = nextMap->getID();
		m_pos.x = m_pos.x - ( curMap.getWidth() * TILE_WIDTH );
		m_pos.y = m_pos.y + ( curMap.getNeighborOffset( Right ) * TILE_HEIGHT );
	}
}

void Character::setMovement( MoveSpeed m, Direction d )
{
	if ( std::get< 2 >( m_move ) && std::get< 0 >( m_move ) == d )
		return;

	std::ostringstream ss;
	ss << strDirection( d ) << "." << strMoveSpeed( m );

	m_sheet.animate( ss.str(), true );
	m_move = std::make_tuple( d, getMoveSpeed( m, d ), false );
}

sf::FloatRect Character::getBounds() const
{
	sf::Vector2i bounds = m_sheet.getDimensions();
	return sf::FloatRect( m_pos.x - ( bounds.x / 2.0f ), m_pos.y - ( bounds.y / 2.0f ), (float) bounds.x, (float) bounds.y );
}

/***************************************************************************/

sf::Sprite Character::toSprite() const
{
	sf::Sprite sprite;

	sprite.setPosition( m_pos );
	m_sheet.update( sprite );

	return sprite;
}

/***************************************************************************/

} // namespace bf
