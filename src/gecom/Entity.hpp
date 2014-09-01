#pragma once

#include <atomic>
#include <memory>
#include <vector>

namespace gecom {

	class Entity {
	private:
		static std::atomic< int > sm_ID;
		int m_ID = 0;
		unsigned char *m_ID_array;
		std::vector< std::weak_ptr< Entity > > m_children;
		void initiliseID();

	public:
		int getID();
		const unsigned char * Entity::getIDByteArray();
		void addChild( std::shared_ptr< Entity > i_child );
		void removeChild( std::shared_ptr< Entity > i_child );
		const std::vector< std::weak_ptr< Entity > > & getChildren();
	};

}