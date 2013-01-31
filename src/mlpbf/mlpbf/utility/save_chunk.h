#pragma once

#include <vector>
#include <sstream>
#include <SFML\System\NonCopyable.hpp>

namespace game
{
	class Saveable;

	//-------------------------------------------------------------------------
	// This class holds a chunk of raw, unformatted data
	// Children SaveChunks exist independent from the parent chunk
	// You can create a Saveable object by calling SaveChunk::Create
	//
	// A const SaveChunk can only be read from
	//
	// FORMAT:
	//	2 bytes - Type of saveable object
	//	4 bytes - Size of custom save area
	//	X bytes - The custom save area, the saved data for each object is here
	//	1 byte - Boolean flag if there are any children
	//		4 bytes - If above is true, the amount of children
	//
	//	If there are children, it repeats the above but adds it the vector of children
	//
	// Notes:
	//	- IMPORTANT: Read in EXACTLY the same order you write
	//	- When you instatiate a SaveChunk for writing, call GetSaveID() in the constructor
	//	- Writing/Reading variables is easy with the template functions Read/Write
	//	- Strings can be written/read to -- DO NOT WRITE char*
	//	- Only write/read primitives (and std::string) -- DO NOT WRITE/READ CLASSES OR char*
	//	- When writing/reading, you're ONLY accessing the custom save area
	//-------------------------------------------------------------------------
	class SaveChunk : public sf::NonCopyable
	{
		public:
			SaveChunk( const Saveable& object );
			SaveChunk( std::istream& input );

			~SaveChunk();

			void addChild( std::unique_ptr< SaveChunk >& child );

			const std::vector< std::unique_ptr< SaveChunk > >& getChildren() const;
			const std::unique_ptr< SaveChunk > operator[]( unsigned i ) const;

			std::unique_ptr< Saveable > create() const;

			void input( std::istream& input );
			void output( std::ostream& output ) const;


			template< typename T >
			const SaveChunk& read( T& data ) const
			{
				m_data.read( (char*) &data, sizeof( T ) );
				return *this;
			}

			template<>
			const SaveChunk& read< std::string >( std::string& data ) const
			{
				std::stringbuf string;
				m_data.get( string, '\0' );
				m_data.get(); // Skip the null terminator
				data = string.str();
				return *this;
			}


			template< typename T >
			SaveChunk& write( const T& data )
			{
				m_data.write( (char*) &data, sizeof( T ) );
				return *this;
			}

			template<>
			SaveChunk& write< std::string >( const std::string& data )
			{
				m_data.write( data.c_str(), ( data.size() + 1 ) * sizeof( char ) );
				return *this;
			}

		private:
			short m_id;
			mutable std::stringstream m_data;
			std::vector< std::unique_ptr< SaveChunk > > m_children;
	};
}