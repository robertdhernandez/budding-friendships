#include "mlpbf/graphics/animation.h"
#include "mlpbf/utility/timer.h"

#include "mlpbf/exception.h"
#include "mlpbf/resource.h"
#include "mlpbf/xml/helper.h"

#include <tinyxml.h>
#include <sstream>

#include <SFML/Graphics/Sprite.hpp>

namespace bf
{
namespace gfx
{

class AttributeTooSmallException : public Exception
{
public:
	AttributeTooSmallException( const std::string& attr )
	{
		*this << "Animation \"" << attr << "\" attribute must be greater than zero";
	}
};

inline int attribute( const TiXmlElement& elem, const std::string& attr )
{
	int val = std::stoi( xml::attribute( elem, attr ) );
	if ( val <= 0 )
		throw AttributeTooSmallException( attr );
	return val;
}

/***************************************************************************/

Animation::Animation( const TiXmlElement& elem ) :
	m_reverse( false ),
	m_flip( false ),
	m_numFrames( 0U ),
	m_frameTime( 0U )
{
	m_id = xml::attribute( elem, "id" );

	m_dim.x = attribute( elem, "width" ); 
	m_dim.y = attribute( elem, "height" );

	m_numFrames = attribute( elem, "frames" );
	m_frameTime = attribute( elem, "frame_time" );

	loadTexture( xml::attribute( elem, "image" ) );

	// Optional

	const char* reverse = elem.Attribute( "reverse" );
	if ( reverse ) std::istringstream( reverse ) >> std::boolalpha >> m_reverse;

	const char* flip = elem.Attribute( "flip" );
	if ( flip ) std::istringstream( flip ) >> std::boolalpha >> m_flip;
}

void Animation::update( sf::Sprite& sprite, unsigned frame ) const
{
	if ( m_reverse ) frame = m_numFrames - frame;
	frame = std::min( m_numFrames - 1, std::max( frame, 0U ) );

	sprite.scale( ( m_flip ) ? -1.0f : 1.0f, 1.0f );
	sprite.setTexture( getTexture() );

	sprite.setOrigin( m_dim.x / 2.0f, m_dim.y / 2.0f );

	// Get the subrect to draw
	sf::IntRect subrect;
	subrect.left	= frame * m_dim.x;
	subrect.top		= 0;
	subrect.width	= m_dim.x;
	subrect.height	= m_dim.y;

	sprite.setTextureRect( subrect );
}

void Animation::setTimer( util::Timer& timer ) const
{
	timer.setTarget( sf::milliseconds( m_frameTime ) );
}

/***************************************************************************/

} // namespace gfx

} // namespace game
