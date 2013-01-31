#include "mlpbf/global.h"
#include "mlpbf/player.h"
#include "mlpbf/direction.h"

#include "mlpbf/item/inventory.h"
#include "mlpbf/resource/loader/texture.h"
#include "mlpbf/ui/window.h"

#include <SFML/Graphics/RenderTarget.hpp>

namespace bf
{

static const std::string INV_SML = "data/ui/inventory/bag_s.png";
static const std::string INV_MED = "data/ui/inventory/bag_m.png";
static const std::string INV_LRG = "data/ui/inventory/bag_l.png";

static const std::string TILE_BG_INNER = "data/ui/inventory/slot_inner.png";
static const std::string TILE_BG_OUTER = "data/ui/inventory/slot_outer.png";

static const float INV_TILE_WIDTH  = 64.0f;
static const float INV_TILE_HEIGHT = 64.0f;
static const float INV_TILE_GAP_H  = 2.0f;
static const float INV_TILE_GAP_V  = 2.0f;

static const sf::Time INV_MOVE_TIME = sf::milliseconds( 350 );

/***************************************************************************/

class Tile : public sf::Drawable, public sf::Transformable, res::TextureLoader< 2 >
{
public:
	Tile( const Item* item ) :
		m_highlight( false ), m_item( item )
	{
		loadTexture( TILE_BG_INNER, TILE_INNER );
		loadTexture( TILE_BG_OUTER, TILE_OUTER );
	}

	const Item* getItem() const
	{
		return m_item;
	}

	void highlight( bool state )
	{
		m_highlight = state;
	}

private:
	enum
	{
		TILE_INNER = 0,
		TILE_OUTER = 1
	};

	void onUpdate()
	{
	}

	void draw( sf::RenderTarget& target, sf::RenderStates states ) const
	{
		states.transform *= getTransform();

		// Draw the outer
		target.draw( sf::Sprite( getTexture( TILE_OUTER ) ), states );

		// Draw the inner
		sf::Sprite inner( getTexture( TILE_INNER ) );
		inner.setPosition( 2.0f, 2.0f );
		inner.setColor( sf::Color( 255, 255, 255, m_highlight ? 200 : 100 ) );

		target.draw( inner, states );

		// Draw the item
		if ( m_item )
		{
			sf::Sprite item( m_item->getIcon() );
			item.setTextureRect( sf::IntRect( 0, 0, (int) INV_TILE_WIDTH, (int) INV_TILE_HEIGHT ) );

			target.draw( item, states );
		}
	}

private:
	bool m_highlight;
	const Item* m_item;
};

/***************************************************************************/

class TileGrid : public ui::Base
{
public:
	TileGrid( const sf::Vector2u& dim )
	{
		const item::Inventory& inv = Player::singleton().getInventory();
		for ( unsigned y = 0; y < dim.y; y++ )
			for ( unsigned x = 0; x < dim.x; x++ )
			{
				unsigned index = y * dim.x + x;
				Item* item = index < inv.getSize() ? inv.getSlot( index ).get() : nullptr;

				std::unique_ptr< Tile > tile( new Tile( item ) );
				tile->setPosition( x * INV_TILE_WIDTH + x * INV_TILE_GAP_H, y * INV_TILE_HEIGHT + y * INV_TILE_GAP_V );

				m_tiles.push_back( std::move( tile ) );
			}
	}

	Tile& getTile( unsigned index )
	{
		return *m_tiles.at( index );
	}

	const Tile& getTile( unsigned index ) const
	{
		return *m_tiles.at( index );
	}

private:
	void onUpdate()
	{
	}

	void draw( sf::RenderTarget& target, sf::RenderStates states ) const
	{
		states.transform *= getTransform();
		for ( auto it = m_tiles.begin(); it != m_tiles.end(); ++it )
			target.draw( **it, states );
	}

private:
	std::vector< std::unique_ptr< Tile > > m_tiles;
};

/***************************************************************************/

class Inventory : public ui::Window
{
public:
	Inventory()
	{
		sf::Vector2u& dim = m_dim;

		switch ( Player::singleton().getInventoryLevel() )
		{
		case 0: loadTexture( INV_SML ); dim.x = 3; dim.y = 1; break;
		case 1: loadTexture( INV_SML ); dim.x = 3; dim.y = 2; break;
		case 2: loadTexture( INV_MED ); dim.x = 3; dim.y = 3; break;
		case 3: loadTexture( INV_MED ); dim.x = 3; dim.y = 4; break;
		case 4: loadTexture( INV_LRG ); dim.x = 4; dim.y = 4; break;
		case 5: loadTexture( INV_LRG ); dim.x = 4; dim.y = 5; break;
		}

		const sf::Vector2u size = getTexture().getSize();
		setOrigin( size.x / 2.0f, size.y / 2.0f );
		setPosition( SCREEN_WIDTH / 2.0f, (float) SCREEN_HEIGHT + size.y );

		TileGrid* grid = new TileGrid( dim );
		grid->setPosition( size.x / 2.0f, size.y / 2.0f );
		grid->setOrigin( ( dim.x * INV_TILE_WIDTH + dim.x * INV_TILE_GAP_H ) / 2.0f, ( dim.y * INV_TILE_HEIGHT + dim.y * INV_TILE_GAP_V ) / 2.0f );

		addChild( grid );
	}

private:
	void onWindowUpdate()
	{
	}

	void onOpen()
	{
		move( sf::Vector2f( SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f ), INV_MOVE_TIME );
		Player::singleton().animate( strDirection( Player::singleton().getDirection() ) + ".bag_open" );
	}

	bool opened() const
	{
		return !isMoving();
	}

	void onClose()
	{
		move( sf::Vector2f( SCREEN_WIDTH / 2.0f, (float) SCREEN_HEIGHT + getTexture().getSize().y ), INV_MOVE_TIME );
		Player::singleton().animate( strDirection( Player::singleton().getDirection() ) + ".bag_close" );
	}

	bool closed() const
	{
		return !isMoving();
	}

	void onKeyPressed( const sf::Event::KeyEvent& ev )
	{
	}

	void onKeyReleased( const sf::Event::KeyEvent& ev )
	{
		switch ( ev.code )
		{
		case sf::Keyboard::A:
			close();
		break;
		}
	}

	sf::Vector2u m_dim;
};

/***************************************************************************/

void showInventory()
{
	if ( ui::Window::getGlobal() )
		throw std::exception( "window in use" );
	ui::Window::setGlobal( new Inventory() );
}

/***************************************************************************/

}