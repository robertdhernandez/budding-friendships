#pragma once

#include <set>
#include <string>

namespace bf
{
	class Map;
	
	namespace gfx
	{
		class Spritesheet;
	}

	namespace data
	{
		struct Item
		{
			std::string id, name, desc, image;
			int buy, sell;
			std::set< std::string > attributes;
		};
	}

	namespace db
	{
		void init();
		void cleanup();
		
		// Returns the item data with inputted id
		const data::Item & getItem( const std::string & id );
		
		// Generates the inputted spritesheet
		void genSprite( const std::string & id, gfx::Spritesheet * sheet );
		
		// Returns the map of string or integer id
		bf::Map & getMap( unsigned id );
		bf::Map & getMap( const std::string & id );
	}
}
