#pragma once

#include <string>
#include <sstream>

namespace bf
{
	class Exception : public std::exception
	{
	public:
		Exception() throw() {}
		Exception( const char * str ) throw() : m_err( str ) {}
		Exception( const std::string & str ) throw() : m_err( str ) {}
		Exception( const std::string && str ) throw() : m_err( str ) {}
		
		virtual ~Exception() throw() {}
	
		const char* what() const throw()
		{
			return m_err.c_str();
		}

		template< typename T >
		Exception& operator<<( const T& t )
		{
			std::ostringstream ss;
			ss << t;
			m_err += ss.str();
			return *this;
		}

	private:
		std::string m_err;
	};
} // namespace bf
