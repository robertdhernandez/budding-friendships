#pragma once

#include "object.h"
#include "../resource/loader/texture.h"

namespace bf
{
	namespace map
	{
		class Sign : public Object, private res::TextureLoader<>
		{
		public:
			void update( sf::Uint32 frameTime, const sf::Vector2f& pos ) {}

			void onEnter( sf::Uint32 frameTime, const sf::Vector2f& pos ) {}
			void onInside( sf::Uint32 frameTime, const sf::Vector2f& pos ) {}
			void onExit( sf::Uint32 frameTime, const sf::Vector2f& pos ) {}

			void onInteract( const sf::Vector2f& pos );
			bool hasCollision( const sf::Vector2f& pos ) const { return true; }

		private:
			void load( const Tmx::Object& object );
			void draw( sf::RenderTarget& target, sf::RenderStates states ) const;

		private:
			std::string m_text;
		};
	}
}