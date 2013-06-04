#pragma once

#include "movespeed.h"
#include "direction.h"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <string>
#include <vector>

namespace bf
{
	namespace time
	{
		class Hour;
	}
	
	class Character;

	class Actor : public virtual sf::NonCopyable
	{
	public:
		Actor() : m_curRepeater( nullptr ) {}
		virtual ~Actor();

		// Moves the character x tiles in a certain direction
		Actor & move( Direction dir, MoveSpeed speed, unsigned tiles );

		// Causes the character to face a certain direction
		Actor & face( Direction dir, bool force = false );

		// Repositions the character (in tiles); optionally in a new map
		Actor & reposition( const sf::Vector2i & pos, const std::string & map = "" );

		// Causes the character to wait a certain amount of time
		Actor & wait( const sf::Time & time );

		// Causes the character to wait until a certain hour
		Actor & wait( const time::Hour & hour );

		// Repeats the next inputted events until a condition is satisified
		Actor & repeatBegin( unsigned numTimes );		// Repeats n times
		Actor & repeatBegin( const time::Hour & hour );	// Repeats until a time

		// Stops adding events to the repeat stack
		Actor & repeatEnd();

		// Returns if there are actions
		bool hasActions() const { return !m_actions.empty(); }

	public:
		class Action;
		class Repeater;

	protected:
		void updateCharacter( Character & );		

	private:
		Actor & addAction( Action* );

	private:
		Repeater * m_curRepeater;
		std::vector< Action * > m_actions;
	};
}
