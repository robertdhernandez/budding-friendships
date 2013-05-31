#pragma once

#include "exception.h"
#include <tinyxml.h>

namespace bf
{
	namespace xml
	{
		// DOCUMENT EXCEPTIONS 

		class DocumentException : public Exception
		{
			public: DocumentException( const TiXmlDocument& xml ) throw();
		};

		class MissingRootElementException : public Exception
		{
			public: MissingRootElementException( const TiXmlDocument& xml ) throw();
		};

		// ELEMENT EXCEPTIONS

		class MissingAttributeException : public Exception
		{
			public: MissingAttributeException( const TiXmlElement& element, const std::string& attribute ) throw();
		};

		class UnrecognisedElementException : public Exception
		{
			public: UnrecognisedElementException( const TiXmlElement& element ) throw();
		};

		class NoChildElementsException : public Exception
		{
			public: NoChildElementsException( const TiXmlElement& element ) throw ();
		};
	
		// This file contains helper functions to make reading XML easier
		// All functions will throw an exception if they fail, so all return values WILL be valid

		// Helper function to open a new XmlDocument
		TiXmlDocument open( const std::string& file ) 
			throw ( DocumentException, MissingRootElementException );

		// Retreive an attribute and checks that it exists
		std::string attribute( const TiXmlElement& element, const std::string& attribute ) 
			throw ( MissingAttributeException );
	}
}
