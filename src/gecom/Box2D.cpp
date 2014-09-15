#include "Box2D.hpp"

std::atomic<uint32_t> gecom::Box2DGameComponent::sm_world_id(1);
std::atomic<uint32_t> gecom::Box2DGameComponent::sm_body_id(1);

// uint32_t createBody(const b2BodyDef& def, std::shared_ptr<B2PhysicsComponent> p)
std::shared_ptr<gecom::B2BodyProxy> gecom::WorldProxy::createBody(const b2BodyDef& def, std::shared_ptr<B2PhysicsComponent> p) {
	//log("phys::WorldProxy::createBody") << "got to line " << __LINE__;
    std::shared_ptr<B2BodyProxy> body = std::make_shared<B2BodyProxy>(m_master->getNewBodyID());//uint32_t n_body_id = m_master->getNewBodyID();
	AsyncExecutor::enqueue(m_master->getThreadID(), [=] { m_master->createBody(m_world_id, body, def); });
	m_bodies[body->getBodyID()] = p;
	return body;
}

void gecom::WorldProxy::createShape(const uint32_t b, const std::shared_ptr<b2Shape>& def) {
	AsyncExecutor::enqueue(m_master->getThreadID(), [=] { m_master->createShape(m_world_id, b, def); });
}

void gecom::WorldProxy::createFixture(const std::shared_ptr<B2BodyProxy>& b, const std::shared_ptr<b2FixtureDef>& def, const std::shared_ptr<b2Shape>& sh) {
	AsyncExecutor::enqueue(m_master->getThreadID(), [=] { m_master->createFixture(m_world_id, b, def, sh); });
}

void gecom::WorldProxy::receivePFO(std::shared_ptr<PhysicsFrame> pfo) {
	//log("phys-thread") << "got that physics frame on the main thread";
	for (auto p : pfo->getAll()) {
		//log("phys-thread") << "p.first: " << p.first << "\tpfo->getAll().count:" << pfo->getAll().size();
		auto target_body = m_bodies.find(p.first);
		if (target_body != m_bodies.end()) {
			target_body->second->recieveFrame(p.second);
		}
	}
}

void gecom::WorldProxy::applyForce(const std::shared_ptr<B2BodyProxy>& b, const i3d::vec3d f) {
	AsyncExecutor::enqueue(m_master->getThreadID(), [=] { m_master->applyForce(b, f); });
}

void gecom::WorldProxy::applyLinearImpulse(const std::shared_ptr<B2BodyProxy>& b, const i3d::vec3d f) {
	AsyncExecutor::enqueue(m_master->getThreadID(), [=] { m_master->applyLinearImpulse(b, f);  });
}