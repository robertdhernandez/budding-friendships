#pragma once

#include "animation.h"
#include "../utility/timer.h"

#include <memory>
#include <string>
#include <unordered_map>

#include <SFML/Graphics/Drawable.hpp>

namespace sf
{
	class Sprite;
}

namespace bf
{
	namespace gfx
	{
		//-------------------------------------------------------------------------
		// This class is a sheet of sprites that can be animated
		// Note: this class contains no rendering information, it is only logic
		//-------------------------------------------------------------------------
		class Spritesheet
		{
		public:
			Spritesheet();
			Spritesheet( const std::string& sprite );

			void addAnimation( const std::string&, std::unique_ptr< Animation > );
			void animate( const std::string& anim, bool loop = true );

			void load( const std::string& sprite );

			bool finished() const;
			operator bool() const { return finished(); }

			const sf::Vector2i& getDimensions() const { return m_curAnim->getDimensions(); }
			sf::Sprite& update( sf::Sprite& ) const;

		public:
			static void setGlobalState( bool state ) { s_active = state; }

		private:
			std::unordered_map< std::string, std::unique_ptr< Animation > > m_animations;
			Animation* m_curAnim;

			bool m_loop;
				
			mutable unsigned m_frame;
			mutable util::Timer m_timer;

		private:
			static bool s_active;
		};
	} // namespace gfx
} // namespace bf