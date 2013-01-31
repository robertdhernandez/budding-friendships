#pragma once

#include <string>
#include <vector>

namespace bf
{
	class Console;

	namespace con
	{
		class Command
		{
		public:
			virtual ~Command() {}

			virtual const std::string name() const = 0;
			virtual void help( Console& ) const = 0;

			bool operator==( const std::string& str ) const { return name() == str; }
			bool operator<( const Command& c ) const { return name() < c.name(); }

			void operator()( Console&, const std::vector< std::string >& ) const;

		private:
			virtual unsigned minArgs() const = 0;
			virtual void execute( Console&, const std::vector< std::string >& ) const = 0;
		};
	}
}