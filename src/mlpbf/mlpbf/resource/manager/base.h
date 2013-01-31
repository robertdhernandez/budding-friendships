#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <SFML/System/NonCopyable.hpp>

namespace bf
{
	namespace res
	{
		//-------------------------------------------------------------------------
		// [TEMPLATE ABSTRACT CLASS]
		//	Simplifies creating a resource manager for each type by defining a general function
		//	An implementation needs only to define _load, which loads the resource which varies on the type
		//	Any class can then access the appropriate resource by calling load
		//
		//	Note: Implementations may throw an exception during loading, so it is important to catch them
		//-------------------------------------------------------------------------
		template< typename T >
		class ResourceManager : private sf::NonCopyable
		{
		public:
			virtual ~ResourceManager() {}

			std::shared_ptr< T > load( const std::string& str )
			{
				auto find = m_data.find( str );
				if ( find != m_data.end() )
				{
					if ( !find->second.expired() )
						return find->second.lock();
					else
					{
						std::shared_ptr< T > val = _load( str );
						find->second = val;
						return val;
					}
				}
				else
				{
					std::shared_ptr< T > val = _load( str );
					m_data.insert( std::make_pair( str, std::weak_ptr< T >( val ) ) );
					return val;
				}
			}

		private:
			virtual std::shared_ptr< T > _load( const std::string& ) const = 0;

		private:
			std::unordered_map< std::string, std::weak_ptr< T > > m_data;
		};
	}
}