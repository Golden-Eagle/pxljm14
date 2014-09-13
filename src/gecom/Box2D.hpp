#ifndef GECOM_BOX2D_HEADER 
#define GECOM_BOX2D_HEADER

#include <thread>
#include <chrono>

#include <Box2D/Box2D.h>

#include "Game.hpp"
#include "Concurrent.hpp"
#include "Entity.hpp"

namespace gecom {
	class PhysicsFrameData;
	class WorldProxy;
	class B2PhysicsComponent;

	class PhysicsFrameData {
		i3d::vec3d pos;
		i3d::vec3d vel;
		double rot;
		double rot_vel;


	public:
		PhysicsFrameData() : pos(i3d::vec3d::zero()), vel(i3d::vec3d::zero()), rot(0), rot_vel(0) { }
		PhysicsFrameData(i3d::vec3d np, i3d::vec3d nv, double nr, double nrv) :
			pos(np), vel(nv), rot(nr), rot_vel(nrv) { }
		i3d::vec3d getPos() { return pos; }
		i3d::vec3d getVel() { return vel; }
		double getRot() { return rot; }
	};



	class PhysicsFrame {
		std::map<uint32_t, PhysicsFrameData> bodies;

	public:
		void add(uint32_t nid, PhysicsFrameData b) {
			bodies[nid] = b;
		}

		std::map<uint32_t, PhysicsFrameData>& getAll() {
			return bodies;
		}
	};

	class Box2DGameComponent : public GameComponent, public std::enable_shared_from_this < Box2DGameComponent > {
		std::thread m_worker;

		// todo: Expose these are options to Box2D Game Component
		int m_velocity_iterations = 6;
		int m_position_iterations = 6;
		float m_time_step = 1.0f / 60.0f;

		static std::atomic<uint32_t> sm_world_id;
		static std::atomic<uint32_t> sm_body_id;
		std::map<uint32_t, std::pair<std::shared_ptr<b2World>, std::shared_ptr<WorldProxy>>> worlds;
		std::map <uint32_t, b2Body*> bodies;

		inline void dowork();

		i3d::vec3d from_b2Vec(b2Vec2 b) {
			return i3d::vec3d(b.x, b.y, 0);
		}

		b2Vec2 to_b2Vec(i3d::vec3d i) {
			return b2Vec2(i.x(), i.y());
		}

	public:
		friend class WorldProxy;
		Box2DGameComponent(Game &g) : GameComponent(g) { }

		uint32_t getNewBodyID() {
			return sm_body_id.fetch_add(1);
		}

		std::thread::id getThreadID() {
			//log("phys-thread::getThreadID()") << m_worker.get_id() << std::endl;
			return m_worker.get_id();
		}

		void createBody(uint32_t n_world_id, uint32_t n_body_id, const b2BodyDef& def) {
			//log("phys::createBody") << "got here!" << std::endl;
			b2Body* w = worlds[n_world_id].first->CreateBody(&def);
			w->SetUserData((void*)n_body_id);
			bodies[n_body_id] = w;
		}

		void createShape(uint32_t w, uint32_t b, const std::shared_ptr<b2Shape>& def) {
			bodies[b]->CreateFixture(def.get(), 0.0f);
		}

		void createFixture(uint32_t w, uint32_t b, const std::shared_ptr<b2FixtureDef>& def, const std::shared_ptr<b2Shape>& sh) {
			bodies[b]->CreateFixture(def.get());
		}

		void applyForce(uint32_t b, i3d::vec3d f) {
			bodies[b]->ApplyForce(to_b2Vec(f), bodies[b]->GetWorldCenter(), true);
		}

		void applyLinearImpulse(uint32_t b, i3d::vec3d f) {
			bodies[b]->ApplyLinearImpulse(to_b2Vec(f), bodies[b]->GetWorldCenter(), true);
		}

		void init() { m_worker = std::thread{ [this]() { this->dowork(); } }; }

		inline std::shared_ptr<WorldProxy> addWorld(i3d::vec3d gravity) {
			auto n_world_id = sm_world_id.fetch_add(1);
			auto n_world = std::make_shared<b2World>(b2Vec2(gravity.x(), gravity.y()));
			auto n_world_proxy = std::make_shared<WorldProxy>(shared_from_this(), n_world_id);
			worlds[n_world_id] = std::make_pair(n_world, n_world_proxy);
			return n_world_proxy;
		}

		void update() {
		}
	};

	class WorldProxy {
		std::shared_ptr<Box2DGameComponent> m_master;
		uint32_t m_world_id;
		std::map<uint32_t, std::shared_ptr<B2PhysicsComponent>> m_bodies;
	public:
		WorldProxy(const std::shared_ptr<Box2DGameComponent>& m, uint32_t nid) : m_master(m), m_world_id(nid) { }

		uint32_t createBody(const b2BodyDef& def, std::shared_ptr<B2PhysicsComponent> p);
		void createFixture(const uint32_t b, const std::shared_ptr<b2FixtureDef>& def, const std::shared_ptr<b2Shape>& sh);
		void createShape(const uint32_t b, const std::shared_ptr<b2Shape>& def);
		void receivePFO(std::shared_ptr<PhysicsFrame> pfo);
		void applyForce(const uint32_t b, const i3d::vec3d f);
		void applyLinearImpulse(const uint32_t b, const i3d::vec3d f);
	};

	class B2PhysicsComponent : public EntityComponent, public std::enable_shared_from_this<B2PhysicsComponent> {
		uint32_t m_b_id;
		std::shared_ptr<WorldProxy> m_world;
	public:
		B2PhysicsComponent(std::shared_ptr<Entity> parent) : EntityComponent(parent) {
			
		}

		virtual void registerWithWorld(std::shared_ptr<WorldProxy> world) {
			m_world = world;

			b2BodyDef def;
			def.type = b2_dynamicBody;
			def.position.Set(getParent()->getPosition().x(), getParent()->getPosition().y());
			m_b_id = world->createBody(def, shared_from_this());

			auto bbb = std::make_shared<b2PolygonShape>();
			bbb->SetAsBox(1, 1);

			auto fix = std::make_shared<b2FixtureDef>();
			fix->shape = bbb.get();
			fix->density = 30;
			fix->friction = 0.6f;

			world->createFixture(m_b_id, fix, bbb);
		}

		void applyForce(i3d::vec3d f) {
			m_world->applyForce(m_b_id, f);
		}

		void applyLinearImpulse(i3d::vec3d f) {
			m_world->applyLinearImpulse(m_b_id, f);
		}

		virtual void recieveFrame(gecom::PhysicsFrameData pfd) {
			// TODO pass by reference more often neo
			//log("phys-test") << pfd.getPos() << std::endl;
			// TODO - NOT THIS

			getParent()->setPosition(pfd.getPos());
			getParent()->setRotation(pfd.getRot());
		}
	};

	class B2PhysicsStatic : public B2PhysicsComponent {
	public:
		B2PhysicsStatic(std::shared_ptr<Entity> parent) : B2PhysicsComponent(parent) {}

		void recieveFrame(gecom::PhysicsFrameData pdf) {
		}

		void registerWithWorld(std::shared_ptr<WorldProxy> world) {
			b2BodyDef def;
			def.type = b2_staticBody;
			def.position.Set(getParent()->getPosition().x(), getParent()->getPosition().y());
			uint32_t bd = world->createBody(def, shared_from_this());

			auto groundBox = std::make_shared<b2PolygonShape>();
			groundBox->SetAsBox(10.0, 1.0f);
			world->createShape(bd, groundBox);
		}
	};

	void Box2DGameComponent::dowork() {
		while (true) {
			auto start = std::chrono::high_resolution_clock::now();

			for (auto world : worlds) {
				auto n_pf = std::make_shared<PhysicsFrame>();
				//log("phys-thread") << "# phys bodies: " << world.second.first->GetBodyCount();
				world.second.first->Step(m_time_step, m_velocity_iterations, m_position_iterations);
				auto body = world.second.first->GetBodyList();
				while (body != nullptr) {
					//log("phys-thread") << "body is not nullptr";
					// GetUserData is set when body is constructed
					// It's a void*
					n_pf->add((uint32_t)body->GetUserData(), PhysicsFrameData(from_b2Vec(body->GetPosition()), from_b2Vec(body->GetLinearVelocity()), body->GetAngle(), body->GetAngularVelocity()));
					body = body->GetNext();
				}

				auto n_world_proxy = world.second.second;
				AsyncExecutor::enqueueMain([=] { n_world_proxy->receivePFO(n_pf); });
			}

			AsyncExecutor::execute(std::chrono::milliseconds(10));

			auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start);
			auto sleep_duration = std::chrono::milliseconds((int)(m_time_step * 1000.0f)) - dur;
			//log("phys-thread") << "Sleeping for " << sleep_duration.count() << std::endl;
			std::this_thread::sleep_for(sleep_duration);
		}
	}
};

#endif