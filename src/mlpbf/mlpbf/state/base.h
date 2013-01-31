#pragma once

#include "../utility/controller/general.h"
#include "../utility/controller/key.h"
#include "../utility/controller/mouse.h"
#include "../utility/controller/text.h"

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

#include <memory>

namespace bf
{
	namespace state
	{
		//-------------------------------------------------------------------------
		// [INTERFACE CLASS]
		// A state is the state of the game running
		// Controls are handled automatically as it inherits every controller
		// Therefore child classes can either inherit a listener and add themself or add or listeners
		//
		// States are globally defined and can be accessed via state::global()
		// To change the global state, call state::global( unique_ptr< state::Base > )
		// The latter returns the previous global state
		//
		// All states are non-copyable
		//
		// VIRTUAL FUNCTIONS
		//
		//	void update()
		//		Updates the state
		//
		//	void draw( sf::RenderTarget&, sf::RenderStates ) const
		//		Draws the state
		//		Inherited from sf::Drawable
		//-------------------------------------------------------------------------
		class Base :
			public sf::Drawable,
			public util::GeneralController,
			public util::KeyController,
			public util::MouseController,
			public util::TextController,
			private sf::NonCopyable
		{
		public:
			virtual ~Base() {}

			void handleEvents( const sf::Event& );
			virtual void update( const sf::Time& ) = 0;
		};

		extern state::Base& global();
		extern std::unique_ptr< state::Base > global( std::unique_ptr< state::Base > );
	}
}