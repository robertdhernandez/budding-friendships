#pragma once

#include <SFML/System/NonCopyable.hpp>
#include <string>
#include <memory>

namespace sf
{
	class Texture;
}

namespace bf
{
	namespace item
	{
		//-------------------------------------------------------------------------
		//	ItemPtr clone() const
		//		Creates and returns a copy of the item
		//		Items cannot use copy constructors and must use clone for copying
		//
		//	void onUsePressed()
		//		Called once when the player presses the use key
		//
		//	void onUseReleased()
		//		Called once when the player releases the use key
		//
		//	const std::string& getName() const
		//		Returns the name of the item
		//
		//	const std::string& getDesc() const
		//		Returns the description text of the item
		//		Can use rich text elements
		//
		//	const std::string& getID() const
		//		Returns the internal game ID of the item
		//
		//	const sf::Texture& getIcon() const
		//		Returns a texture to display as the item icon
		//		The texture must have a width and height <= 64; any higher will be cut-off
		//
		//	unsigned getBuy() const
		//		Returns the purchasing value of the item
		//
		//	unsigned getSell() const
		//		Returns the selling value of the item
		//		A value of 0 marks an item as unsellable
		//
		//	bool canRemove() const
		//		Signals the parent inventory to remove the item on the next update pass
		//-------------------------------------------------------------------------
		class Base : public sf::NonCopyable
		{
		public:
			virtual ~Base() {}

			virtual std::shared_ptr< Base > clone() const = 0;

			virtual void onUsePressed() = 0;
			virtual void onUseReleased() = 0;

			virtual const std::string& getName() const = 0;
			virtual const std::string& getDesc() const = 0;
			virtual const std::string& getID() const = 0;

			virtual const sf::Texture& getIcon() const = 0;

			virtual unsigned getBuy() const = 0;
			virtual unsigned getSell() const = 0;

			virtual bool canRemove() const = 0;
		};
	}

	typedef item::Base Item;
	typedef std::shared_ptr< Item > ItemPtr;
}