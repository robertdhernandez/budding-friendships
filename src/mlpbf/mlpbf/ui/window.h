#pragma once

#include "base.h"
#include "../resource.h"
#include "../utility/listener/key.h"

#include <memory>
#include <vector>

namespace bf
{
	namespace ui
	{
		class Window : 
			public Base, 
			public util::KeyListener,
			protected res::TextureLoader<>
		{
		public:
			Window();
			virtual ~Window() {}

			void close();
			bool canRemove() const;

			bool isClosing() const { return m_state == Closing; }

		public:
			static Window* getGlobal();
			static std::unique_ptr< Window > setGlobal( Window* );

		protected:
			void addChild( ui::Base* );
			ui::Base& getChild( unsigned index );

		private:
			void onUpdate();

			virtual void onWindowUpdate() = 0;

			// Functions called when the container is opening/closing; returns true when finished
			virtual void onOpen() = 0;			// Called once to initialize
			virtual bool opened() const = 0;	// Called multiple times to determine if finished

			virtual void onClose() = 0;			// Called once to initialize
			virtual bool closed() const = 0;	// Called multiple times to determine if finished

			void draw( sf::RenderTarget&, sf::RenderStates ) const;

		private:
			bool m_init;
			enum { Opening, Opened, Closing, Closed } m_state;
			std::vector< std::unique_ptr< ui::Base > > m_children;
		};
	}
}
