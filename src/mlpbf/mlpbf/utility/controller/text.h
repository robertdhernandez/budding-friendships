#pragma once

#include <SFML/Window/Event.hpp>

namespace bf
{
	namespace util
	{
		class TextListener;

		class TextController
		{
		public:
			TextController() : m_textListener( nullptr ) {}
			virtual ~TextController() { removeTextListener(); }

			void setTextListener( TextListener& );
			void removeTextListener();

		protected:
			void updateTextListener( const sf::Event& );

		private:
			TextListener* m_textListener;
		};
	}
}