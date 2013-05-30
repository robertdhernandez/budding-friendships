#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/NonCopyable.hpp>
#include <Tmx.h>

#include "direction.h"
#include "time/season.h"

namespace bf
{
	class Character;

	class Map : private sf::NonCopyable
	{
	public:
		~Map();
	
		class Object;
	
		void load( unsigned id, const std::string& );
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

		const std::vector< Map::Object * >& getObjects() const { return m_objects; }

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
		std::vector< Map::Object * > m_objects;
		std::vector< Map::Object * > m_activeObjects;
	};
	
	class MapViewer : public sf::Drawable, public sf::Transformable
	{
	public:
		MapViewer( const Map & map );
		virtual ~MapViewer() {}

		void addCharacter( const Character & c ) { m_characters.push_back( &c ); }

		void center( const sf::Vector2f& pos );
		const sf::Vector2f center() const;

		void dimension( const sf::Vector2f & dim );
		const sf::Vector2f dimension() const;

		void map( const Map& map ) { m_map = &map; }
		const Map & map() const { return *m_map; }

		const sf::FloatRect& getViewArea() const { return m_area; }

	protected:
		virtual void draw( sf::RenderTarget&, sf::RenderStates ) const;

	private:
		const Map * m_map;
		sf::FloatRect m_area;

		std::vector< const Character* > m_characters;
	};

	class MultiMapViewer : public MapViewer
	{
	public:
		MultiMapViewer( const Map & m ) : MapViewer( m ) {}

	private:
		void draw( sf::RenderTarget&, sf::RenderStates ) const;
	};
}
