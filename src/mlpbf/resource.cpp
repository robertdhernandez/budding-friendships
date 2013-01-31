#include "mlpbf/resource/manager/font.h"
#include "mlpbf/resource/manager/sound.h"
#include "mlpbf/resource/manager/music.h"
#include "mlpbf/resource/manager/texture.h"

#include "mlpbf/resource/loader/texture.h"

#include "mlpbf/exception.h"

#include <sstream>

namespace bf
{
namespace res
{

/***************************************************************************/

class TextureLoadException : public Exception
{
public:
	TextureLoadException( const std::string& file )
	{
		*this << "Failed to load texture \"" << file << "\"";
	}
};

TextureManager& TextureManager::singleton()
{
	static TextureManager manager;
	return manager;
}

std::shared_ptr< sf::Texture > TextureManager::_load( const std::string& file ) const
{
	std::shared_ptr< sf::Texture > res( new sf::Texture() );
	if ( !res->loadFromFile( file ) )
		throw TextureLoadException( file );
	return res;
}

/***************************************************************************/

class FontLoadException : public Exception
{
public:
	FontLoadException( const std::string& file )
	{
		*this << "Failed to load font \"" << file << "\"";
	}
};

FontManager& FontManager::singleton()
{
	static FontManager manager;
	return manager;
}

std::shared_ptr< sf::Font > FontManager::_load( const std::string& file ) const
{
	std::shared_ptr< sf::Font > res( new sf::Font() );
	if ( !res->loadFromFile( file ) )
		throw FontLoadException( file );
	return res;
}

/***************************************************************************/

class SoundLoadException : public Exception
{
public:
	SoundLoadException( const std::string& file )
	{
		*this << "Failed to load sound \"" << file << "\"";
	}
};

SoundManager& SoundManager::singleton()
{
	static SoundManager manager;
	return manager;
}

std::shared_ptr< sf::SoundBuffer > SoundManager::_load( const std::string& file ) const
{
	std::shared_ptr< sf::SoundBuffer > res( new sf::SoundBuffer() );
	if ( !res->loadFromFile( file ) )
		throw SoundLoadException( file );
	return res;
}

/***************************************************************************/

class MusicLoadException : public Exception
{
public:
	MusicLoadException( const std::string& file )
	{
		*this << "Failed to load music \"" << file << "\"";
	}
};

MusicManager& MusicManager::singleton()
{
	static MusicManager manager;
	return manager;
}

std::shared_ptr< sf::Music > MusicManager::_load( const std::string& file ) const
{
	std::shared_ptr< sf::Music > res( new sf::Music() );
	if ( !res->openFromFile( file ) )
		throw MusicLoadException( file );
	return res;
}

/***************************************************************************/

} // namespace res

} // namespace bf