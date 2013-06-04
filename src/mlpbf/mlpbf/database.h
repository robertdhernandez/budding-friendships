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
			string id;	// internal ID to be referenced by
			string name;	// name string
			string desc; 	// description string
			string image;	// string path to image
			unsigned buy;	// purchase value
			unsigned sell; // selling value
			std::set< string > attributes; // set of attributes
		};
		
		struct Crop
		{
			string id;			// internal ID to be referenced by
			const Item * seed;		// item of the seed
			const Item * crop; 		// item of the crop
			string image;			// string path to image
			time::Seasons seasons;	// season(s) to grow in
			unsigned regrowth;		// index of growth stage to revert once harvest -- 0 mean single harvest
			std::vector< unsigned > growth; // vector of length of growth for each stage
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
