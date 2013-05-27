#include "mlpbf/resource.h"
#include "mlpbf/exception.h"

#include <cassert>
#include <memory>
#include <unordered_map>
#include <string>
#include <sstream>
#include <SFML/System/NonCopyable.hpp>

namespace bf
{
namespace res
{

/***************************************************************************/

class TextureLoadException : public Exception { public: TextureLoadException( const std::string & file ) throw() { *this << "Failed to load texture \"" << file << "\""; } };
class FontLoadException : public Exception { public: FontLoadException( const std::string & file ) { *this << "Failed to load font \"" << file << "\""; } };
class SoundLoadException : public Exception { public: SoundLoadException( const std::string & file ) { *this << "Failed to load sound \"" << file << "\""; } };
class MusicLoadException : public Exception { public: MusicLoadException( const std::string & file ) { *this << "Failed to load music \"" << file << "\""; } };

/***************************************************************************/

template< typename T >
class ResourceManager : private sf::NonCopyable
{
public:
	virtual ~ResourceManager() {}

	std::shared_ptr< T > load( const std::string & str )
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
	virtual std::shared_ptr< T > _load( const std::string & ) const = 0;

private:
	std::unordered_map< std::string, std::weak_ptr< T > > m_data;
};

class FontManager : public ResourceManager< sf::Font >
{
	std::shared_ptr< sf::Font > _load( const std::string & file ) const
	{
		std::shared_ptr< sf::Font > res( new sf::Font() );
		if ( !res->loadFromFile( file ) )
			throw FontLoadException( file );
		return res;
	}
} * g_FontManager = NULL;

class MusicManager : public ResourceManager< sf::Music >
{
	std::shared_ptr< sf::Music > _load( const std::string & file ) const
	{
		std::shared_ptr< sf::Music > res( new sf::Music() );
		if ( !res->openFromFile( file ) )
			throw MusicLoadException( file );
		return res;
	}
} * g_MusicManager = NULL;

class SoundManager : public ResourceManager< sf::SoundBuffer >
{
	std::shared_ptr< sf::SoundBuffer > _load( const std::string & file ) const
	{
		std::shared_ptr< sf::SoundBuffer > res( new sf::SoundBuffer() );
		if ( !res->loadFromFile( file ) )
			throw SoundLoadException( file );
		return res;
	}
} * g_SoundManager = NULL;

class TextureManager : public ResourceManager< sf::Texture >
{
	std::shared_ptr< sf::Texture > _load( const std::string& file ) const
	{
		std::shared_ptr< sf::Texture > res( new sf::Texture() );
		if ( !res->loadFromFile( file ) )
			throw TextureLoadException( file );
		return res;
	}
} * g_TextureManager = NULL;

/***************************************************************************/

void init()
{
	g_FontManager		= new FontManager();
	g_MusicManager		= new MusicManager();
	g_SoundManager		= new SoundManager();
	g_TextureManager	= new TextureManager();
}

void cleanup()
{
	delete g_FontManager;
	delete g_MusicManager;
	delete g_SoundManager;
	delete g_TextureManager;
	
	g_FontManager 		= NULL;
	g_MusicManager 	= NULL;
	g_SoundManager 	= NULL;
	g_TextureManager 	= NULL;
}

/***************************************************************************/

FontPtr loadFont( const std::string & str )
{
	assert( g_FontManager != NULL );
	return g_FontManager->load( str );
}

MusicPtr loadMusic( const std::string & str )
{
	assert( g_MusicManager != NULL );
	return g_MusicManager->load( str );
}

SoundBufferPtr loadSound( const std::string & str )
{
	assert( g_SoundManager != NULL );
	return g_SoundManager->load( str );
}

TexturePtr loadTexture( const std::string & str )
{
	assert( g_TextureManager != NULL );
	return g_TextureManager->load( str );
}

/***************************************************************************/

} // namespace res

} // namespace bf
