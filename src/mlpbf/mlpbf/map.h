#pragma once

#include <array>
#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <string>
#include <Tmx.h>
#include <unordered_map>
#include <memory>

#include "map/object.h"

namespace bf
{
	namespace time
	{
		enum Season;
	}

	enum Direction;

	class Map : private sf::NonCopyable
	{
	public:
		void load( unsigned, const std::string& );
		void loadNeighbors();

		void update( sf::Uint32 frameTime, const sf::Vector2f& pos );
		bool interact( const sf::Vector2f& pos );

		bool checkTileCollision( const sf::Vector2u& ) const;
		bool checkObjectCollision( const sf::Vector2f& ) const;

		void season( time::Season s ) { m_season = s; }
		time::Season season() const { return m_season; }

	public: // Functions to help with rendering
		unsigned getWidth() const { return m_map.GetWidth(); }
		unsigned getHeight() const { return m_map.GetHeight(); }

		unsigned getID() const { return m_mapID; }

		const std::vector< std::unique_ptr< map::Object > >& getObjects() const { return m_objects; }

		const std::vector< const Tmx::Layer* >& getLowerLayers() const { return m_lower; }
		const std::vector< const Tmx::Layer* >& getUpperLayers() const { return m_upper; }

		const Tmx::Layer* getCollisionLayer() const { return m_collision; }

		bf::Map* getNeighbor( Direction d ) { return m_neighbors[ d ].first; }
		int getNeighborOffset( Direction d ) { return m_neighbors[ d ].second; }

		const bf::Map* getNeighbor( Direction d ) const { return m_neighbors[ d ].first; }
		int getNeighborOffset( Direction d ) const { return m_neighbors[ d ].second; }

		bool adjustSprite( const Tmx::Layer& layer, sf::Vector2u pos, sf::Sprite& ) const;

	public: // Global variable
		static Map& global();
		static Map& global( unsigned id );
		static Map& global( const std::string& map );
		
	private:
		Tmx::Map m_map;
		unsigned m_mapID;

		time::Season m_season;

		const Tmx::Layer* m_collision;
		std::vector< const Tmx::Layer* > m_lower, m_upper;
		std::unordered_map< const Tmx::Tileset*, std::shared_ptr< sf::Texture > > m_textures;

		std::array< std::pair< bf::Map*, int >, 4 > m_neighbors;

		// Map Objects
		std::vector< std::unique_ptr< map::Object > > m_objects;
		std::vector< map::Object* > m_activeObjects;
	};
}