#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>

#include <string>
#include <memory>

namespace Tmx
{
	class Object;
};

namespace bf
{
	namespace map
	{
		//-------------------------------------------------------------------------
		// A map object is an abstract class for an object that appears on a map
		// The object must implement methods when the player interacts with the object in several ways
		// 
		// FUNCTION EXPLANATIONS
		//
		//		const sf::FloatRect& getBounds() const
		//			Returns the FloatRect of the object's bounds
		//			NOTE: left != getPosition().x and top != getPosition().y
		//
		//	IMPLEMENTABLE METHODS
		//
		//		void load( const Tmx::Object&, Instance&, Map& )
		//			Called once when the object is being created
		//			Retrieve references to external classes here and load from data from the TMX object
		//
		//		void update( sf::Uint32, const sf::Vector2f& )
		//			Called continiously every frame regardless if the player is inside the object
		//			NOTE: the coordinate inputted is relative to the object
		//
		//		void onEnter( sf::Uint32, const sf::Vector2f& )
		//			Called once when the player enters the object
		//			NOTE: the coordinate inputted is relative to the object
		//
		//		void onInside( sf::Uint32, const sf::Vector2f& )
		//			Called continuously while the player is inside the object
		//			NOTE: the coordinate inputted is relative to the object
		//
		//		void onExit( sf::Uint32, const sf::Vector2f& )
		//			Called once when the player exits the object
		//			NOTE: the coordinate inputted is relative to the object
		//
		//		void onInteract( const sf::Vector2f& )
		//			Called once when the player interacts with the object with the primary key (default: z)
		//			Takes in the absolute coordinate that was interacted with
		//			NOTE: the coordinate inputted is relative to the object
		//
		//		bool hasCollision( const sf::Vector2f& )
		//			Returns if the position at the inputted absolute coordinate has collision
		//			NOTE: the coordinate inputted is relative to the object
		//-------------------------------------------------------------------------
		class Object : public sf::Drawable, private sf::Transformable
		{
		public:
			static std::unique_ptr< Object > create( const Tmx::Object& object );
			virtual ~Object() {}

			const std::string& getName() const { return m_name; }
			const sf::FloatRect& getBounds() const { return m_bounds; }

			using sf::Transformable::getPosition;
			using sf::Transformable::setPosition;

		public:
			virtual void load( const Tmx::Object& object ) = 0;
			virtual void update( sf::Uint32 frameTime, const sf::Vector2f& pos ) = 0;

			virtual void onEnter( sf::Uint32 frameTime, const sf::Vector2f& pos ) = 0;
			virtual void onInside( sf::Uint32 frameTime, const sf::Vector2f& pos ) = 0;
			virtual void onExit( sf::Uint32 frameTime, const sf::Vector2f& pos ) = 0;

			virtual void onInteract( const sf::Vector2f& pos ) = 0;

			virtual bool hasCollision( const sf::Vector2f& pos ) const = 0;

		protected:
			using sf::Transformable::getTransform;

		private:
			std::string m_name;
			sf::FloatRect m_bounds;
		};
	}
}