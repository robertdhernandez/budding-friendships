#pragma once

#include <string>

namespace bf
{
	enum
	{
		SCREEN_WIDTH = 800,
		SCREEN_HEIGHT = 600
	};

	enum
	{
		TILE_WIDTH = 32,
		TILE_HEIGHT = 32
	};

	extern bool DEBUG_COLLISION;
	extern bool SHOW_FPS;

	void showText( const std::string& message, const std::string& speaker = "" );
	void showInventory();
}
