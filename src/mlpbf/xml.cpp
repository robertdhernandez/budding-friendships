#include "mlpbf/xml.h"

#include <sstream>
#include <tinyxml.h>

namespace bf
{

/***************************************************************************/

TiXmlDocument xml::open( const std::string& filename )
	throw ( DocumentException, MissingRootElementException )
{
	TiXmlDocument xml( filename.c_str() );
	if ( !xml.LoadFile() ) throw xml::DocumentException( xml );
	if ( xml.RootElement() == nullptr ) throw xml::MissingRootElementException( xml );
	return xml;
}

std::string xml::attribute( const TiXmlElement& element, const std::string& attribute )
	throw ( MissingAttributeException )
{
	const char* id = element.Attribute( attribute.c_str() );
	if ( !id ) throw xml::MissingAttributeException( element, attribute );
	return std::string( id );
}

/***************************************************************************/

xml::DocumentException::DocumentException( const TiXmlDocument& xml ) throw()
{
	*this << "Error reading XML file \"" << xml.Value() << "\" at line " << xml.ErrorRow() << ": " << xml.ErrorDesc();
}

xml::MissingAttributeException::MissingAttributeException( const TiXmlElement& element, const std::string& attribute ) throw()
{
	*this << "Element <" << element.Value() << "> is missing attribute \"" << attribute << "\" on line " << element.Row();
}

xml::MissingRootElementException::MissingRootElementException( const TiXmlDocument& xml ) throw()
{
	*this << "XML file \"" << xml.Value() << "\" is missing the root element";
}

xml::UnrecognisedElementException::UnrecognisedElementException( const TiXmlElement& element ) throw()
{
	*this << "Unrecognised element <" << element.Value() << "> on line " << element.Row();
}

xml::NoChildElementsException::NoChildElementsException( const TiXmlElement& element ) throw()
{
	*this << "Element <" << element.Value() << "> on line " << element.Row() << " must contain child elements";
}

/***************************************************************************/

} // namespace game
