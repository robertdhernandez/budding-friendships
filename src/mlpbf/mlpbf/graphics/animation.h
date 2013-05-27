#pragma once

#include "../resource.h"
#include <SFML/Graphics/Sprite.hpp>

class TiXmlElement;

namespace bf
{
	namespace util
	{
		class Timer;
	}

	namespace gfx
	{
		//-------------------------------------------------------------------------
		// This struct contains simple data for an animation
		//	num_frames: the number of frames to animate
		//	frame_time: the number of ms between each frame
		//	dim:		the dimension of a single frame
		//	reverse:	optional boolean value to play the animation backwards
		//	flip:		optional boolean value to flip animation (for left-right)
		//-------------------------------------------------------------------------
		class Animation : private res::TextureLoader<>
		{
			public:
				Animation( const TiXmlElement& elem );

				const std::string& getID() const { return m_id; }
				unsigned getNumFrames() const { return m_numFrames; }
				const sf::Vector2i& getDimensions() const { return m_dim; }

				void update( sf::Sprite& sprite, unsigned frame ) const;
				void setTimer( util::Timer& ) const;

			private:
				std::string m_id;
				bool m_reverse, m_flip;
				unsigned m_numFrames, m_frameTime;
				sf::Vector2i m_dim;
		};
	}
}

