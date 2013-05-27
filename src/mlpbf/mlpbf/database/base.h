#pragma once

#include "../xml/helper.h"

#include <SFML/System/NonCopyable.hpp>
#include <memory>
#include <string>
#include <tinyxml.h>
#include <unordered_map>

namespace bf
{
	template< typename T >
	class Database : public sf::NonCopyable
	{
	public:
		virtual ~Database() {}

		const T& get( const std::string& id ) const
		{
			auto find = m_data.find( id );
			if ( find == m_data.end() )
				throw InvalidElementException( id );
			return *find->second.get();
		}

		const T& operator[]( const std::string& id ) const 
		{
			return get( id );
		}
		
		class InvalidElementException : public std::exception
		{
			std::string m_err;
		
		public:
			InvalidElementException( const std::string & id ) throw()
			{
				m_err = "Cannot find " + id;
			}
			
			~InvalidElementException() throw() {}
			
			const char * what() const throw() { return m_err.c_str(); }
		};

	protected:
		Database() {}

		void init()
		{
			TiXmlDocument xml = xml::open( getSourceFile() );
			const TiXmlElement& root = *xml.RootElement();
			
			const std::string element = getElementType();

			const TiXmlNode* it = nullptr;
			while ( it = root.IterateChildren( element.c_str(), it ) )
			{
				const TiXmlElement& elem = static_cast< const TiXmlElement& >( *it );

				std::string id = xml::attribute( elem, "id" );

				std::unique_ptr< T > entry( new T() );
				load( elem, *entry );

				m_data.insert( std::make_pair( id, std::move( entry ) ) );
			}
		}

	private:
		virtual const std::string getSourceFile() const = 0;
		virtual const std::string getDatabaseName() const = 0;
		virtual const std::string getElementType() const = 0;

		virtual void load( const TiXmlElement&, T& ) const = 0;

	private:
		std::unordered_map< std::string, std::unique_ptr< T > > m_data;
	};
}
