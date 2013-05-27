#pragma once

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <array>
#include <cassert>
#include <memory>
#include <string>

namespace bf
{
	namespace res
	{
		void init();
		void cleanup();
		
		typedef std::shared_ptr< sf::Font > 		FontPtr;
		typedef std::shared_ptr< sf::Music > 		MusicPtr;
		typedef std::shared_ptr< sf::SoundBuffer > 	SoundBufferPtr;
		typedef std::shared_ptr< sf::Texture > 		TexturePtr;
		
		FontPtr		loadFont( const std::string & filename );
		MusicPtr		loadMusic( const std::string & filename );
		SoundBufferPtr	loadSound( const std::string & filename );
		TexturePtr	loadTexture( const std::string & filename );
		
		template< std::size_t Size = 1 >
		class FontLoader
		{
			std::array< FontPtr, Size > m_font;
		
		public:
			virtual ~FontLoader() {}

			void loadFont( const std::string& file, std::size_t index = 0 ) { m_font.at( index ) = res::loadFont( file ); }

			sf::Font& getFont( std::size_t index = 0 ) { assert( m_font[ index ] ); return *m_font[ index ]; }
			const sf::Font& getFont( std::size_t index = 0 ) const { assert( m_font[ index ] ); return *m_font[ index ]; }
		};
		
		template< std::size_t Size = 1 >
		class TextureLoader
		{
			std::array< TexturePtr, Size > m_texture;
			
		public:
			virtual ~TextureLoader() {}

			void loadTexture( const std::string& file, std::size_t index = 0 ) 
			{ 
				m_texture.at( index ) = res::loadTexture( file );
			}

			sf::Texture& getTexture( std::size_t index = 0 ) { assert( m_texture[ index ] ); return *m_texture[ index ]; }
			const sf::Texture& getTexture( std::size_t index = 0 ) const { assert( m_texture[ index ] ); return *m_texture[ index ]; }
		};
	}
}
