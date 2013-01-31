#include "mlpbf/map/object.h"
#include "mlpbf/map/sign.h"

#include "mlpbf/global.h"
#include "mlpbf/exception.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <TmxObject.h>

namespace bf
{
namespace map
{

class UnknownObjectException : public Exception { public: UnknownObjectException( const Tmx::Object& object ) { *this << "unknown object type \"" << object.GetType() << "\""; } };

/***************************************************************************/

std::unique_ptr< Object > Object::create( const Tmx::Object& tmxObject )
{
	std::unique_ptr< Object > object;

	std::string type = tmxObject.GetType();
	std::transform( type.begin(), type.end(), type.begin(), ::tolower );

	if ( type == "sign" )
		object.reset( new Sign() );
	else
		throw UnknownObjectException( tmxObject );
	
	object->setPosition( (float) tmxObject.GetX(), (float) tmxObject.GetY());

	object->m_name = tmxObject.GetName();
	object->m_bounds = sf::FloatRect( (float) tmxObject.GetX(), (float) tmxObject.GetY(), (float) tmxObject.GetWidth(), (float) tmxObject.GetHeight() );

	object->load( tmxObject );
	return object;
}

/***************************************************************************/
//	mlpbf/map/door.h

// TODO

/***************************************************************************/
//	mlpbf/map/sign.h

void Sign::load( const Tmx::Object& object )
{
	const auto& list = object.GetProperties().GetList();

	//auto findTexture = list.find( "texture" );
	//auto findText = list.find( "text" );

	//TODO: error checking

	loadTexture( list.at( "texture" ) );
	m_text = list.at( "text" );
}

void Sign::onInteract( const sf::Vector2f& pos )
{
	showText( m_text );
}

void Sign::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{
	states.transform *= getTransform();
	const sf::FloatRect& rect = getBounds();

	sf::Sprite sprite;
	sprite.setTexture( getTexture() );
	sprite.setTextureRect( sf::IntRect( 0, 0, (int) rect.width, (int) rect.height ) );

	target.draw( sprite, states );
}

/***************************************************************************/

} // namespace map

} // namespace bf