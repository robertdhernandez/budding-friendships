#include "mlpbf/utility/controller/general.h"
#include "mlpbf/utility/controller/key.h"
#include "mlpbf/utility/controller/mouse.h"
#include "mlpbf/utility/controller/text.h"

#include "mlpbf/utility/listener/general.h"
#include "mlpbf/utility/listener/key.h"
#include "mlpbf/utility/listener/mouse.h"
#include "mlpbf/utility/listener/text.h"

namespace bf
{
namespace util
{

using sf::Event;

/***************************************************************************/

void GeneralListener::unregisterGeneral()
{
	if ( m_generalParent )
		m_generalParent->removeGeneralListener();
}

void GeneralController::setGeneralListener( GeneralListener& l )
{
	l.unregisterGeneral();
	l.m_generalParent = this;
	m_generalListener = &l;
}

void GeneralController::removeGeneralListener()
{
	if ( m_generalListener )
	{
		m_generalListener->m_generalParent = nullptr;
		m_generalListener = nullptr;
	}
}

void GeneralController::updateGeneralListener( const Event& ev )
{
	if ( m_generalListener )
		switch ( ev.type )
		{
		case Event::GainedFocus:	m_generalListener->onGainedFocus(); break;
		case Event::LostFocus:		m_generalListener->onLostFocus(); break;
		case Event::Resized:		m_generalListener->onResize( ev.size ); break;
		default: break;
		}
}

/***************************************************************************/

void KeyListener::unregisterKey()
{
	if ( m_keyParent )
		m_keyParent->removeKeyListener();
}

void KeyController::setKeyListener( KeyListener& key )
{
	key.unregisterKey();
	key.m_keyParent = this;
	m_keyListener = &key;
}

void KeyController::removeKeyListener()
{
	if ( m_keyListener )
	{
		m_keyListener->m_keyParent = nullptr;
		m_keyListener = nullptr;
	}
}

void KeyController::updateKeyListener( const Event& ev )
{
	if ( m_keyListener )
		switch ( ev.type )
		{
		case Event::KeyPressed:	 m_keyListener->onKeyPressed( ev.key ); break;
		case Event::KeyReleased: m_keyListener->onKeyReleased( ev.key ); break;
		default: break;
		}
}

/***************************************************************************/

void MouseListener::unregisterMouse()
{
	if ( m_mouseParent )
		m_mouseParent->removeMouseListener();
}

void MouseController::setMouseListener( MouseListener& mouse )
{
	mouse.unregisterMouse();
	mouse.m_mouseParent = this;
	m_mouseListener = &mouse;
}

void MouseController::removeMouseListener()
{
	if ( m_mouseListener )
	{
		m_mouseListener->m_mouseParent = nullptr;
		m_mouseListener = nullptr;
	}
}

void MouseController::updateMouseListener( const Event& ev )
{
	if ( m_mouseListener )
		switch ( ev.type )
		{
		case Event::MouseButtonPressed:		m_mouseListener->onMouseButtonPressed( ev.mouseButton ); break;
		case Event::MouseButtonReleased:	m_mouseListener->onMouseButtonReleased( ev.mouseButton ); break;
		case Event::MouseEntered:			m_mouseListener->onMouseEntered(); break;
		case Event::MouseLeft:				m_mouseListener->onMouseLeft(); break;
		case Event::MouseMoved:				m_mouseListener->onMouseMoved( ev.mouseMove ); break;
		case Event::MouseWheelMoved:		m_mouseListener->onMouseWheelMoved( ev.mouseWheel ); break;
		default: break;
		}
}

/***************************************************************************/

void TextListener::unregisterText()
{
	if ( m_textParent )
		m_textParent->removeTextListener();
}

void TextController::setTextListener( TextListener& text )
{
	text.unregisterText();
	text.m_textParent = this;
	m_textListener = &text;
}

void TextController::removeTextListener()
{
	if ( m_textListener )
	{
		m_textListener->m_textParent = nullptr;
		m_textListener = nullptr;
	}
}

void TextController::updateTextListener( const sf::Event& ev )
{
	if ( m_textListener && ev.type == Event::TextEntered ) 
		m_textListener->onTextEntered( ev.text );
}

/***************************************************************************/

} // namespace util

} // namespace cw
