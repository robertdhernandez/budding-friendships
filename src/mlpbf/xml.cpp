#include "mlpbf/xml/exception.h"
#include "mlpbf/xml/helper.h"

#include <sstream>
#include <tinyxml.h>

namespace bf
{
namespace xml
{

/***************************************************************************/

TiXmlDocument open( const std::string& filename )
{
	TiXmlDocument xml( filename.c_str() );
	if ( !xml.LoadFile() ) throw xml::DocumentException( xml );
	if ( xml.RootElement() == nullptr ) throw xml::MissingRootElementException( xml );
	return xml;
}

std::string attribute( const TiXmlElement& element, const std::string& attribute )
{
	const char* id = element.Attribute( attribute.c_str() );
	if ( !id ) throw xml::MissingAttributeException( element, attribute );
	return std::string( id );
}

/***************************************************************************/

DocumentException::DocumentException( const TiXmlDocument& xml ) throw()
{
	*this << "Error reading XML file \"" << xml.Value() << "\" at line " << xml.ErrorRow() << ": " << xml.ErrorDesc();
}

MissingAttributeException::MissingAttributeException( const TiXmlElement& element, const std::string& attribute ) throw()
{
	*this << "Element <" << element.Value() << "> is missing attribute \"" << attribute << "\" on line " << element.Row();
}

MissingRootElementException::MissingRootElementException( const TiXmlDocument& xml ) throw()
{
	*this << "XML file \"" << xml.Value() << "\" is missing the root element";
}

UnrecognisedElementException::UnrecognisedElementException( const TiXmlElement& element ) throw()
{
	*this << "Unrecognised element <" << element.Value() << "> on line " << element.Row();
}

NoChildElementsException::NoChildElementsException( const TiXmlElement& element ) throw()
{
	*this << "Element <" << element.Value() << "> on line " << element.Row() << " must contain child elements";
}

/***************************************************************************/

} // namespace xml

} // namespace game