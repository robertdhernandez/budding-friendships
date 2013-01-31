#pragma once

#include <SFML/Window/Event.hpp>

namespace bf
{
	namespace util
	{
		//-------------------------------------------------------------------------
		// All listeners inherit from this general listener class
		// It gives access to basic events, which by default do nothing
		//-------------------------------------------------------------------------
		class GeneralListener
		{
		public:
			GeneralListener() : m_generalParent( nullptr ) {}
			virtual ~GeneralListener() { unregisterGeneral(); }

			friend class GeneralController;

			virtual void onLostFocus() = 0;
			virtual void onGainedFocus() = 0;
			virtual void onResize( const sf::Event::SizeEvent& ) = 0;

		protected:
			void unregisterGeneral();

		private:
			GeneralController* m_generalParent;
		};
	}
}