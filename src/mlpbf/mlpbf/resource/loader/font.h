#pragma once

#include <array>
#include <cassert>
#include "../manager/font.h"

namespace bf
{
	namespace res
	{
		template< std::size_t Size = 1 >
		class FontLoader
		{
		public:
			virtual ~FontLoader() {}

			void loadFont( const std::string& file, std::size_t index = 0 ) { m_font[ index ] = FontManager::singleton().load( file ); }

			sf::Font& getFont( std::size_t index = 0 ) { assert( m_font[ index ] ); return *m_font[ index ]; }
			const sf::Font& getFont( std::size_t index = 0 ) const { assert( m_font[ index ] ); return *m_font[ index ]; }

		private:
			std::array< std::shared_ptr< sf::Font >, Size > m_font;
		};
	}
}