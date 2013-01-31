#pragma once

#include "base.h"
#include <SFML/Audio/Music.hpp>

namespace bf
{
	namespace res
	{
		//-------------------------------------------------------------------------
		// [SINGLETON CLASS]
		//	Implementation of ResourceManager using sf::Music
		//	Can be globally accessed with getSingleton()
		//
		//	Defined in: ResourceManager.cpp
		//-------------------------------------------------------------------------
		class MusicManager : public ResourceManager< sf::Music >
		{
		public:
			static MusicManager& singleton();

		private:
			MusicManager() {}
			std::shared_ptr< sf::Music > _load( const std::string& ) const;
		};
	}
}