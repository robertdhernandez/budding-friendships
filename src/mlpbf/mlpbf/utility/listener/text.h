#pragma once

#include <SFML/Window/Event.hpp>

namespace bf
{
	namespace util
	{
		class TextController;
	
		//-------------------------------------------------------------------------
		// Classes that implement this interface can be interacted via text input
		//-------------------------------------------------------------------------
		class TextListener
		{
		public:
			TextListener() : m_textParent( nullptr ) {}
			virtual ~TextListener() { unregisterText(); }

			friend class TextController;

			virtual void onTextEntered( const sf::Event::TextEvent& ) = 0;

		protected:
			void unregisterText();

		private:
			TextController* m_textParent;
		};
	}
}
