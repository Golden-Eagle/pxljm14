#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

namespace gecom {

	class Entity {
		using entity_id_t = uint32_t;
	private:
		static std::atomic< entity_id_t > sm_ID;
		entity_id_t m_ID = 0;
		unsigned char *m_ID_array;
		std::vector< std::weak_ptr< Entity > > m_children;
		void initiliseID();

	public:
		entity_id_t getID();
		const unsigned char * Entity::getIDByteArray();
		void addChild( std::shared_ptr< Entity > i_child );
		void removeChild( std::shared_ptr< Entity > i_child );
		const std::vector< std::weak_ptr< Entity > > & getChildren();
	};

}