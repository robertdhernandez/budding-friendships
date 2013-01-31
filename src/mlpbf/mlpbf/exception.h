#pragma once

#include <string>
#include <sstream>

namespace bf
{
	class Exception : public std::exception
	{
	public:
		virtual ~Exception() {}

		const char* what() const
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