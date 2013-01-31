#pragma once

#include "base.h"
#include <SFML/Audio/SoundBuffer.hpp>

namespace bf
{
	namespace res
	{
		//-------------------------------------------------------------------------
		// [SINGLETON CLASS]
		//	Implementation of ResourceManager using sf::SoundBuffer
		//	This only returns a SoundBuffer so the user must have their own sf::Sound to use the buffer
		//	Can be globally accessed with getSingleton()
		//
		//	Defined in: ResourceManager.cpp
		//-------------------------------------------------------------------------
		class SoundManager : public ResourceManager< sf::SoundBuffer >
		{
		public:
			static SoundManager& singleton();

		private:
			SoundManager() {}
			std::shared_ptr< sf::SoundBuffer > _load( const std::string& ) const;
		};
	}
}