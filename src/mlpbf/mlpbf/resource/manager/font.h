#pragma once

#include "base.h"
#include <SFML/Graphics/Font.hpp>

namespace bf
{
	namespace res
	{
		//-------------------------------------------------------------------------
		// [SINGLETON CLASS]
		//	Implementation of ResourceManager using sf::Font
		//	Can be globally accessed with getSingleton()
		//
		//	Defined in: ResourceManager.cpp
		//-------------------------------------------------------------------------
		class FontManager : public ResourceManager< sf::Font >
		{
		public:
			static FontManager& singleton();

		private:
			FontManager() {}
			std::shared_ptr< sf::Font > _load( const std::string& ) const;
		};
	}
}