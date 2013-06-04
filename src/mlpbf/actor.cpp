#include "mlpbf/actor.h"
#include "mlpbf/character.h"

#include "mlpbf/global.h"
#include "mlpbf/direction.h"
#include "mlpbf/time.h"

#include "mlpbf/map.h"
#include "mlpbf/database.h"

#include "mlpbf/utility/timer.h"

#include <algorithm>
#include <functional>
#include <tuple>

namespace bf
{

/***************************************************************************/

class Actor::Action
{
public:
	Action() : m_init( false ) {}
	virtual ~Action() {}

	bool execute( Character & c )
	{
		if ( !m_init )
		{
			m_init = true;
			init( c );
		}
		return play( c );
	}
	void reinit() { m_init = false; }

private:
	virtual void init( Character& ) = 0;
	virtual bool play( Character& ) = 0;

private:
	bool m_init;
};

class Actor::Repeater : public Actor::Action
{
public:
	Repeater( Repeater * last ) : 
		m_lastRepeater( last ), 
		m_index( 0U ) 
	{}
	
	virtual ~Repeater() 
	{ 
		for ( Action * a : m_actions ) 
			delete a; 
	}

	void addAction( Action * action ) 
	{
		m_actions.push_back( action ); 
	}
	
	Repeater * getLastRepeater() 
	{ 
		return m_lastRepeater;
	}

private:
	void init( Character & )
	{
		m_index = 0U;
		onInit();
	}
	
	bool play( Character & c )
	{
		while ( m_index < m_actions.size() && m_actions[ m_index ]->execute( c ) )
		{
			m_actions[ m_index ]->reinit();
			if ( ++m_index == m_actions.size() )
			{
				if ( canFinish() )
					return true;
				else
					m_index = 0;
			}
		}

		return false;
	}

	virtual void onInit() = 0;
	virtual bool canFinish() = 0;

private:
	Repeater * const m_lastRepeater;
	std::size_t m_index;
	std::vector< Action * > m_actions;
};

/***************************************************************************/

inline float distance( const sf::Vector2f& a, const sf::Vector2f& b )
{
	return std::sqrt( std::pow( a.x - b.x, 2 ) + std::pow( a.y - b.y, 2 ) );
}

class Move : public Actor::Action
{
public:
	Move( Direction d, MoveSpeed m, unsigned tiles ) :
		m_dir( d ), m_speed( m ), m_tiles( tiles ), m_distance( 0.0f )
	{
	}

private:
	void init( Character& c )
	{
		m_distance = 0.0f;
		m_last = std::make_tuple( c.getMapID(), c.getPosition() );
		c.setMovement( m_speed, m_dir );

		// Calculate the destination position
		m_destPos = c.getPosition();
		switch ( m_dir )
		{
		case Up:	m_destPos.y -= m_tiles * TILE_HEIGHT; break;
		case Down:	m_destPos.y += m_tiles * TILE_HEIGHT; break;
		case Left:	m_destPos.x -= m_tiles * TILE_WIDTH;  break;
		case Right:	m_destPos.x += m_tiles * TILE_WIDTH;  break;
		}
	}

	bool play( Character& c )
	{
		if ( c.getMapID() == std::get< 0 >( m_last ) )
			m_distance += distance( std::get< 1 >( m_last ), c.getPosition() );
		else
		{
			const Map& lastMap = db::getMap( std::get< 0 >( m_last ) );
			const sf::Vector2f& lastPos = std::get< 1 >( m_last );
			sf::Vector2f pos( lastPos );

			switch ( c.getDirection() )
			{
			case Up:
				pos.y = ( lastMap.getNeighbor( Up )->getHeight() * TILE_HEIGHT - lastPos.y ) - c.getPosition().y;
				m_destPos.y += lastMap.getHeight() * TILE_HEIGHT;
			break;

			case Down:
				pos.y = c.getPosition().y + lastMap.getHeight() * TILE_HEIGHT;
				m_destPos.y -= lastMap.getHeight() * TILE_HEIGHT;
			break;

			case Left:
				pos.x = ( lastMap.getNeighbor( Left )->getWidth() * TILE_WIDTH - lastPos.x ) - c.getPosition().x;
				m_destPos.x += lastMap.getWidth() * TILE_WIDTH;
			break;

			case Right:
				pos.x = c.getPosition().x + lastMap.getWidth() * TILE_WIDTH;
				m_destPos.x -= lastMap.getWidth() * TILE_WIDTH;
			break;
			}

			m_distance += distance( lastPos, pos );
		}

		float size = ( m_dir == Left || m_dir == Right ) ? (float) TILE_WIDTH  : (float) TILE_HEIGHT;

		m_last = std::make_tuple( c.getMapID(), c.getPosition() );
		bool complete = m_distance >= m_tiles * size;

		if ( complete )
		{
			c.setPosition( m_destPos );
			c.setMovement( Idle, m_dir );
		}

		return complete;
	}

private:
	const Direction m_dir;
	const MoveSpeed m_speed;
	const unsigned m_tiles;

	std::tuple< unsigned, sf::Vector2f > m_last;
	float m_distance;
	sf::Vector2f m_destPos;
};

/***************************************************************************/

class Face : public Actor::Action
{
public:
	Face( Direction d ) : m_dir( d )
	{
	}

private:
	void init( Character& c )
	{
		c.setMovement( Idle, m_dir );
	}

	bool play( Character& c )
	{
		return true;
	}

private:
	const Direction m_dir;
};

/***************************************************************************/

class Reposition : public Actor::Action
{
public:
	Reposition( const sf::Vector2i& pos, const std::string& map ) :
		m_pos( pos.x * TILE_WIDTH + ( TILE_WIDTH / 2.0f ), pos.y * TILE_HEIGHT + ( TILE_HEIGHT / 2.0f ) ), 
		m_map( map )
	{
	}

private:
	void init( Character& c )
	{
		if ( m_map.empty() )
			c.setPosition( m_pos );
		else
			c.setMap( m_map, m_pos );
	}

	bool play( Character& c )
	{
		return true;
	}

private:
	const sf::Vector2f m_pos;
	const std::string m_map;
};

/***************************************************************************/

class WaitTime : public Actor::Action
{
public:
	WaitTime( const sf::Time& time )
	{
		m_timer.setTarget( time );
	}

private:
	void init( Character& )
	{
		m_timer.restart();
		m_timer.setState( true );
	}

	bool play( Character& )
	{
		return m_timer.finished();
	}

private:
	util::Timer m_timer;
};

class WaitHour : public Actor::Action
{
public:
	WaitHour( const time::Hour& hour ) :
		m_hour( hour )
	{
	}

private:
	void init( Character& )
	{
	}

	bool play( Character& )
	{
		return Time::singleton().getHour() >= m_hour;
	}

private:
	const time::Hour m_hour;
};

/***************************************************************************/

class RepeaterNumTimes : public Actor::Repeater
{
public:
	RepeaterNumTimes( Repeater* last, unsigned numTimes ) :
		Repeater( last ),
		m_numTimes( numTimes ),
		m_loops( 0U )
	{
	}

private:
	void onInit()
	{
		m_loops = 0U;
	}

	bool canFinish()
	{
		return ++m_loops == m_numTimes;
	}

private:
	const unsigned m_numTimes;
	unsigned m_loops;
};

/***************************************************************************/

Actor::~Actor()
{
	for ( Action * a : m_actions )
		delete a;
}

Actor & Actor::move( Direction dir, MoveSpeed speed, unsigned tiles )
{
	return addAction( new Move( dir, speed, tiles ) );
}

Actor & Actor::face( Direction dir, bool force )
{
	//TODO: force direction
	return addAction( new Face( dir ) );
}

Actor & Actor::reposition( const sf::Vector2i & pos, const std::string & map )
{
	return addAction( new Reposition( pos, map ) );
}

Actor & Actor::wait( const sf::Time & time )
{
	return addAction( new WaitTime( time ) );
}

Actor & Actor::wait( const time::Hour & hour )
{
	return addAction( new WaitHour( hour ) );
}

Actor & Actor::repeatBegin( unsigned numTimes )
{
	RepeaterNumTimes* a = new RepeaterNumTimes( m_curRepeater, numTimes );
	addAction( a );
	m_curRepeater = a;
	return *this;
}

Actor & Actor::repeatEnd()
{
	assert( m_curRepeater != nullptr );
	m_curRepeater = m_curRepeater->getLastRepeater();
	return *this;
}

void Actor::updateCharacter( Character & c )
{
	while ( !m_actions.empty() && ( *m_actions.begin() )->execute( c ) )
		m_actions.erase( m_actions.begin() );
}

Actor & Actor::addAction( Action * action )
{
	if ( m_curRepeater )
		m_curRepeater->addAction( action );
	else
		m_actions.push_back( action );
	return *this;
}

/***************************************************************************/

} // namespace bf
