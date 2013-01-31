#pragma once

#include "../exception.h"

class TiXmlDocument;
class TiXmlElement;

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
	} // namespace xml
} // namespace game