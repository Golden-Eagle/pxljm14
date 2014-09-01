
#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

#include "Entity.hpp"

namespace gecom {

	std::atomic< Entity::entity_id_t > Entity::sm_ID(1);

	void Entity::initiliseID() {
		if ( m_ID == 0 ) {
			m_ID = sm_ID.fetch_add( 1 );

			m_ID_array = new unsigned char[ 4 ];

			m_ID_array[0] = (m_ID >> 24) & 0xFF;
			m_ID_array[1] = (m_ID >> 16) & 0xFF;
			m_ID_array[2] = (m_ID >> 8) & 0xFF;
			m_ID_array[3] = m_ID & 0xFF;
		}
	}

	Entity::entity_id_t Entity::getID() {
		initiliseID();
		return m_ID;
	}

	const unsigned char * Entity::getIDByteArray() {
		initiliseID();
		return m_ID_array;
	}

	void Entity::addChild( std::shared_ptr< Entity > i_child ) {
		m_children.push_back(std::weak_ptr< Entity >(i_child));
	}

	void Entity::removeChild( std::shared_ptr< Entity > i_child ) {
		//TODO
	}

	const std::vector< std::weak_ptr< Entity > > & Entity::getChildren() {
		return m_children;
	}

}