#pragma once

#include "base.h"
#include <SFML/Graphics/Texture.hpp>

namespace bf
{
	namespace res
	{
		//-------------------------------------------------------------------------
		// [SINGLETON CLASS]
		//	Implementation of ResourceManager using sf::Texture
		//	Can be globally accessed with getSingleton()
		//
		//	Defined in: ResourceManager.cpp
		//-------------------------------------------------------------------------
		class TextureManager : public ResourceManager< sf::Texture >
		{
		public:
			static TextureManager& singleton();

		private:
			TextureManager() {}
			std::shared_ptr< sf::Texture > _load( const std::string& ) const;
		};
	}
}