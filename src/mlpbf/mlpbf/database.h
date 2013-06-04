#pragma once

#include "time/season.h"
#include <set>
#include <string>
#include <vector>

namespace bf
{
	class Map;
	
	namespace gfx
	{
		class Spritesheet;
	}

	namespace data
	{
		using std::string;
	
		struct Item
		{
			string id; 
			string name;
			string desc; 
			string image;
			unsigned buy;
			unsigned sell;
			std::set< std::string > attributes;
		};
		
		struct Crop
		{
			string id;
			const Item * seed;
			const Item * crop; 
			string image;
			time::Seasons seasons;
			unsigned regrowth;
			std::vector< unsigned > growth;
		};
	}

	namespace db
	{
		void init();
		void cleanup();
		
		// Returns the item data with inputted id
		const data::Item & getItem( const std::string & id );
		
		// Return the crop data with the inputted id
		const data::Crop & getCrop( const std::string & id );
		
		// Generates the inputted spritesheet
		void genSprite( const std::string & id, gfx::Spritesheet * sheet );
		
		// Returns the map of string or integer id
		bf::Map & getMap( unsigned id );
		bf::Map & getMap( const std::string & id );
	}
}
