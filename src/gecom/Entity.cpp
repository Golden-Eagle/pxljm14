#include "Entity.hpp"
#include "Box2D.hpp"
#include "Scene.hpp"
#include <atomic>

std::atomic<gecom::entity_id_t> gecom::Entity::sm_ID(1);

void gecom::Entity::addChild(std::shared_ptr<gecom::Entity>& i_child) {
	AsyncExecutor::enqueueMain([=] {
		m_world->getScene()->add(i_child); 
		m_children.push_back(i_child);
	});
	
}