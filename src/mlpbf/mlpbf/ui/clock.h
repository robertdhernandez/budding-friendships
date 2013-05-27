#include "base.h"
#include "../resource.h"

namespace bf
{
	namespace time
	{
		class Date;
		class Hour;
	}

	namespace ui
	{
		class Clock : public Base, private res::TextureLoader< 3 >, private res::FontLoader<>
		{
		public:
			Clock( const time::Date&, const time::Hour& );

			void setVisible( bool visible );
			bool isVisible() const;

		private:
			void onUpdate();
			void draw( sf::RenderTarget&, sf::RenderStates ) const;

		private:
			bool m_visible;
			const time::Date& m_date;
			const time::Hour& m_hour;
		};
	}
}
