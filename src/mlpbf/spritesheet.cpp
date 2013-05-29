#include "mlpbf/graphics/spritesheet.h"
#include "mlpbf/database.h"

#include "mlpbf/exception.h"
#include <sstream>

#include <SFML/Graphics/Sprite.hpp>

namespace bf
{

namespace gfx
{

bool Spritesheet::s_active = true;

class AnimationNotFoundException : public Exception
{
public:
	AnimationNotFoundException( const std::string& str )
	{
		*this << "Animation \"" << str << "\" does not exist!";
	}
};

/***************************************************************************/

Spritesheet::Spritesheet() :
	m_curAnim( nullptr ),
	m_loop( false )
{
}

Spritesheet::Spritesheet( const std::string& sprite ) :
	m_curAnim( nullptr ),
	m_loop( false )
{
	load( sprite );
}

/***************************************************************************/

void Spritesheet::addAnimation( const std::string& str, std::unique_ptr< Animation > anim )
{
	//TODO: check if already exists
	m_animations.insert( std::make_pair( str, std::move( anim ) ) );
}

void Spritesheet::animate( const std::string& anim, bool loop )
{
	auto find = m_animations.find( anim );
	if ( find == m_animations.end() )
		throw AnimationNotFoundException( anim );

	m_curAnim = find->second.get();
	m_loop = loop;
	m_frame = 0;

	m_timer.restart();
	m_curAnim->setTimer( m_timer );
}

void Spritesheet::load( const std::string& sprite )
{
	m_animations.clear();
	db::genSprite( sprite, this );
}

bool Spritesheet::finished() const
{
	if ( m_loop )
		return false;
	else
		return m_curAnim != nullptr && m_timer && m_frame == m_curAnim->getNumFrames() - 1;
}

/***************************************************************************/

sf::Sprite& Spritesheet::update( sf::Sprite& sprite ) const
{
	if ( m_curAnim )
	{
		const Animation& anim = *m_curAnim;

		if ( s_active != m_timer.getState() )
			m_timer.setState( s_active );

		// Timer has finished
		if ( s_active && m_timer )
		{
			if ( m_loop )
				m_frame = ( m_frame + 1 ) % anim.getNumFrames();
			else if ( m_frame < anim.getNumFrames() )
				m_frame++;

			m_timer.restart();
		}

		anim.update( sprite, m_frame );
	}

	return sprite;
}

/***************************************************************************/

} // namespace gfx

} // namespace game
