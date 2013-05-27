#pragma once

#include "movespeed.h"
#include "direction.h"

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <memory>
#include <string>
#include <vector>

namespace bf
{
	namespace time
	{
		class Hour;
	}
	
	class Character;

	class Actor
	{
	public:
		Actor() : m_curRepeater( nullptr ) {}
		virtual ~Actor() {}

		// Moves the character x tiles in a certain direction
		Actor& move( Direction dir, MoveSpeed speed, unsigned tiles );

		// Causes the character to face a certain direction
		Actor& face( Direction dir, bool force = false );

		// Repositions the character (in tiles); optionally in a new map
		Actor& reposition( const sf::Vector2i& pos, const std::string& map = "" );

		// Causes the character to wait a certain amount of time
		Actor& wait( const sf::Time& time );

		// Causes the character to wait until a certain hour
		Actor& wait( const time::Hour& hour );

		// Repeats the next inputted events until a condition is satisified
		Actor& repeatBegin( unsigned numTimes );		// Repeats n times
		Actor& repeatBegin( const time::Hour& hour );	// Repeats until a time

		// Stops adding events to the repeat stack
		Actor& repeatEnd();

		// Returns if there are actions
		bool hasActions() const { return !m_actions.empty(); }

	public:
		class Action
		{
		public:
			Action() : m_init( false ) {}
			virtual ~Action() {}

			bool execute( Character& c );
			void reinit() { m_init = false; }

		private:
			virtual void init( Character& ) = 0;
			virtual bool play( Character& ) = 0;

		private:
			bool m_init;
		};

		class Repeater : public Action
		{
		public:
			Repeater( Repeater* last ) : m_lastRepeater( last ), m_index( 0U ) {}
			virtual ~Repeater() {}

			void addAction( Action* action ) { m_actions.push_back( std::unique_ptr< Action >( action ) ); }
			Repeater* getLastRepeater() { return m_lastRepeater; }

		private:
			void init( Character& );
			bool play( Character& );

			virtual void onInit() = 0;
			virtual bool canFinish() = 0;

		private:
			Repeater* const m_lastRepeater;
			std::size_t m_index;
			std::vector< std::unique_ptr< Action > > m_actions;
		};

	protected:
		void updateCharacter( Character& );		

	private:
		Actor& addAction( Action* );

	private:
		Repeater* m_curRepeater;
		std::vector< std::unique_ptr< Action > > m_actions;
	};
}
