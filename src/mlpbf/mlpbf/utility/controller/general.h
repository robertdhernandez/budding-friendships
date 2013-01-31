#pragma once

#include <SFML/Window/Event.hpp>

namespace bf
{
	namespace util
	{
		class GeneralListener;

		class GeneralController
		{
		public:
			GeneralController() : m_generalListener( nullptr ) {}
			virtual ~GeneralController() { removeGeneralListener(); }

			void setGeneralListener( GeneralListener& );
			void removeGeneralListener();

		protected:
			void updateGeneralListener( const sf::Event& );

		private:
			GeneralListener* m_generalListener;
		};
	}
}