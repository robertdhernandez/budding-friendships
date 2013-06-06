#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/NonCopyable.hpp>
#include <string>
#include <vector>

namespace bf
{
	class Item : sf::NonCopyable
	{
	public:
		virtual Item * clone() const = 0;
		virtual ~Item() {}
		
		virtual void use() = 0;
		
		virtual const std::string & getName() const = 0;
		virtual const std::string & getDesc() const = 0;
		virtual const std::string & getID() const = 0;
		
		virtual unsigned getBuy() const = 0;
		virtual unsigned getSell() const = 0;
		
		virtual bool canRemove() const = 0;
	};
	
	Item * generateItem( const std::string & id, unsigned char quantity = 1U, unsigned char quality = 100U );
	
	class Inventory
	{
		bool m_hasLimit;
		std::vector< Item * > m_items;
	
	public:
		Inventory( unsigned size = 0U );
		Inventory( const Inventory & copy );
		Inventory & operator=( const Inventory & copy );
		~Inventory();
		
		void addItem( Item * item );
		void addItem( Item * item, unsigned index );
		
		Item * getItem( unsigned index );
		const Item * getItem( unsigned index ) const;
		
		Item * removeItem( unsigned index );
		
		unsigned getSize() const;
		void setSize( unsigned size );
	};
}
