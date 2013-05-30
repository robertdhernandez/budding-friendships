#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/NonCopyable.hpp>

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
				WIDTH = 40,
				HEIGHT = 40
			};
			
			class Object
			{
			public:
				virtual ~Object() {}
				
				virtual bool hasCollision() const { return true; }
				
				virtual unsigned getWidth() const { return 1; }
				virtual unsigned getHeight() const { return 1; }
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
			const Tile & getTile( unsigned x, unsigned y );
			
			// Returns the tile array -- size == field::WIDTH * field::HEIGHT
			const Tile * getTiles();
			
			void plant( unsigned x, unsigned y, const Seed & seed );
		}
	}
}
