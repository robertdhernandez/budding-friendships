#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/NonCopyable.hpp>
#include <vector>

namespace bf
{
	class Seed;

	namespace farm
	{
		void init();
		void cleanup();
	
		namespace field
		{
			enum
			{
				WIDTH = 20,
				HEIGHT = 20
			};
			
			class Object : public sf::Drawable, sf::Transformable
			{
			public:
				virtual ~Object() {}
				
				virtual bool hasCollision() const = 0;
				
				virtual unsigned getWidth() const = 0;
				virtual unsigned getHeight() const = 0;
				
				using sf::Transformable::getPosition;
				using sf::Transformable::setPosition;
				
				using sf::Transformable::getTransform;
			};
			
			class Tile : sf::NonCopyable
			{
				Tile() : object( nullptr ), till( 0U ), water( false ), highlight( false ) {}
				~Tile() {}
			
			public:
				field::Object * object;
				unsigned char till;
				bool water;
				bool highlight;
				
				friend void farm::init();
				friend void farm::cleanup();
			};
			
			// Returns an individual tile
			Tile & getTile( unsigned x, unsigned y );
			
			// Returns the tile array -- size == field::WIDTH * field::HEIGHT
			Tile * getTiles();
			
			// Returns the object array -- size param gets set to size of the array
			const std::vector< Object * > & getObjects();
			
			// Places a stone -- size must be [1,3] and area must be empty
			void placeStone( unsigned x, unsigned y, unsigned size );
			
			// Plants a seed
			void plantSeed( unsigned x, unsigned y, const Seed & seed );
		}
	}
}
