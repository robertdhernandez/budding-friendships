#pragma once

#include <memory>

namespace game
{
	class SaveChunk;

	//-------------------------------------------------------------------------
	// An interface for a class that can be saved
	// It must override the two methods for saving and loading
	// Saving returns a SaveChunk pointer
	// Loading modifies the current class with a SaveChunk reference
	//-------------------------------------------------------------------------
	class Saveable
	{
		public:
			virtual ~Saveable() {}

			virtual std::unique_ptr< SaveChunk > save() const = 0;

			virtual void load( const SaveChunk& data ) = 0;
			virtual short getSaveID() const = 0;

		public:
			enum ID
			{
				UNKNOWN = 0,

				DATE = 1,
				HOUR = 2,

				FORECAST = 3,

				PLAYER = 4,
				INVENTORY = 5,
				ITEM = 6,
				
				NPC_DATA = 7,
				NPC_MANAGER = 8,

				FIELD = 10,
				FIELD_TILE = 11,

				FIELD_BIN = 12,
				FIELD_CROP = 13,
				FIELD_POND = 14,
				FIELD_STONE = 15,
				FIELD_WEED = 16,

				TOOL_HOE = 30,
				TOOL_HAMMER = 31,
				TOOL_WATER = 32,
				TOOL_PLANTER = 33,
			};
	};
}