#pragma once

#include <array>
#include <cassert>
#include "../manager/texture.h"

namespace bf
{
	namespace res
	{
		//-------------------------------------------------------------------------
		// [HELPER CLASS]
		//	Allows child classes to easily load and manage their texture
		//
		//	Defined in: ResourceLoader.cpp
		//-------------------------------------------------------------------------
		template< std::size_t Size = 1 >
		class TextureLoader
		{
		public:
			virtual ~TextureLoader() {}

			void loadTexture( const std::string& file, std::size_t index = 0 ) { m_texture[ index ] = TextureManager::singleton().load( file ); }

			sf::Texture& getTexture( std::size_t index = 0 ) { assert( m_texture[ index ] ); return *m_texture[ index ]; }
			const sf::Texture& getTexture( std::size_t index = 0 ) const { assert( m_texture[ index ] ); return *m_texture[ index ]; }

		private:
			std::array< std::shared_ptr< sf::Texture >, Size > m_texture;
		};
	}
}