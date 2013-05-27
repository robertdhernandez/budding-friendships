#include "mlpbf/state/base.h"
#include "mlpbf/utility/listener/key.h"

#include "mlpbf/console.h"
#include "mlpbf/exception.h"

namespace bf
{

static std::unique_ptr< state::Base > GLOBAL_STATE( nullptr );

/***************************************************************************/

state::Base& state::global()
{
	if ( !GLOBAL_STATE )
		throw Exception( "No state loaded!" );
	return *GLOBAL_STATE;
}

std::unique_ptr< state::Base > bf::state::global( std::unique_ptr< state::Base > state )
{
	std::unique_ptr< state::Base > temp = std::move( GLOBAL_STATE );
	GLOBAL_STATE = std::move( state );
	return temp;
}

void state::Base::handleEvents( const sf::Event& ev )
{
	updateGeneralListener( ev );
	updateKeyListener( ev );
	updateMouseListener( ev );
	updateTextListener( ev );
}

/***************************************************************************/

} // namespace bf
