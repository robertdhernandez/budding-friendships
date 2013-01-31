#pragma once

#include "base.h"

namespace bf
{	
	namespace gfx
	{
		class Spritesheet;
	}

	namespace data
	{
		class Sprite
		{
		public:
			void load( const TiXmlElement& );
			void generate( gfx::Spritesheet& ) const;

		private:
			std::string m_file;
		};
	}

	namespace db
	{
		class Sprite : public Database< data::Sprite >
		{
		public:
			static db::Sprite& singleton();
			
		private:
			Sprite() { init(); }

			const std::string getSourceFile() const { return "data/sprites.xml"; }
			const std::string getDatabaseName() const { return "sprite database"; }
			const std::string getElementType() const { return "sprite"; }

			void load( const TiXmlElement& elem, data::Sprite& sprite ) const { sprite.load( elem ); }
		};
	}
}