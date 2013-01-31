#pragma once

#include <string>

class TiXmlDocument;
class TiXmlElement;

namespace bf
{
	namespace xml
	{
		// This file contains helper functions to make reading XML easier
		// All functions will throw an exception if they fail, so all return values WILL be valid

		// Helper function to open a new XmlDocument
		TiXmlDocument open( const std::string& file );

		// Retreive an attribute and checks that it exists
		std::string attribute( const TiXmlElement& element, const std::string& attribute );

	} // namespace xml
} // namespace bf