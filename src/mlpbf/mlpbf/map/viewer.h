#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <vector>

namespace Tmx
{
	class Layer;
}

namespace bf
{
	class Character;
	class Map;

	namespace map
	{
		class Viewer : public sf::Drawable, public sf::Transformable
		{
		public:
			Viewer( const Map& map );
			virtual ~Viewer() {}

			void addCharacter( const Character& c ) { m_characters.push_back( &c ); }

			void center( const sf::Vector2f& pos );
			const sf::Vector2f center() const;

			void dimension( const sf::Vector2f& dim );
			const sf::Vector2f dimension() const;

			void map( const Map& map ) { m_map = &map; }
			const Map& map() const { return *m_map; }

			const sf::FloatRect& getViewArea() const { return m_area; }

		protected:
			virtual void draw( sf::RenderTarget&, sf::RenderStates ) const;

		private:
			const Map* m_map;
			sf::FloatRect m_area;

			std::vector< const Character* > m_characters;
		};

		class MultiViewer : public Viewer
		{
		public:
			MultiViewer( const Map& m ) : Viewer( m ) {}

		private:
			void draw( sf::RenderTarget&, sf::RenderStates ) const;
		};
	}
}