#pragma once

#include "actor.h"
#include "direction.h"
#include "movespeed.h"
#include "graphics/spritesheet.h"
#include "utility/listener/key.h"

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/NonCopyable.hpp>
#include <tuple>

namespace bf
{
	class Map;

	//TODO: reorganize functions
	class Character : public Actor, public virtual sf::NonCopyable
	{
	public:
		Character( const std::string& spritesheet );

		void animate( const std::string& anim, bool loop = false ) { m_sheet.animate( anim, loop ); }

		void update( const sf::Time& );

		void setMovement( MoveSpeed, Direction );
		bool isMoving() const { return std::get< 1 >( m_move ) != sf::Vector2f( 0.0f, 0.0f ); }

		sf::FloatRect getBounds() const;
		Direction getDirection() const { return std::get< 0 >( m_move ); }

		const sf::Vector2f& getPosition() const { return m_pos; }
		void setPosition( const sf::Vector2f& pos ) { m_pos = pos; }
		
		inline void enableCollision( bool b) { m_checkCollision = b; }

		void setMap( const std::string& map );
		void setMap( const std::string& map, const sf::Vector2f& pos );

		unsigned getMapID() const { return m_mapID; }

		sf::Sprite toSprite() const;
		operator sf::Sprite() const { return toSprite(); }

	private:
		gfx::Spritesheet m_sheet;

		unsigned m_mapID;
		sf::Vector2f m_pos;
		std::tuple< Direction, sf::Vector2f, bool > m_move;
		
		bool m_checkCollision;
	};
}
