#ifndef GECOM_BOX2D_HEADER 
#define GECOM_BOX2D_HEADER

#include <thread>
#include <chrono>

#include <Box2D/Box2D.h>

#include "Game.hpp"
#include "GameComponent.hpp"
#include "Concurrent.hpp"
#include "Entity.hpp"

#define FOOT_SENSOR 1238123


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

	inline i3d::vec3d from_b2Vec(b2Vec2 b) {
		return i3d::vec3d(b.x, b.y, 0);
	}

	inline b2Vec2 to_b2Vec(i3d::vec3d i) {
		return b2Vec2(i.x(), i.y());
	}

    class B2BodyProxy {
        uint32_t m_body_id;

    public:
        B2BodyProxy(uint32_t bid) : m_body_id(bid) { }
        uint32_t getBodyID() { return m_body_id; }
    };

	class Box2DGameComponent : public GameComponent, public std::enable_shared_from_this <Box2DGameComponent> {
		std::thread m_worker;

		// todo: Expose these are options to Box2D Game Component
		int m_velocity_iterations = 6;
		int m_position_iterations = 6;
		float m_time_step = 1.0f / 60.0f;

		static std::atomic<uint32_t> sm_world_id;
		static std::atomic<uint32_t> sm_body_id;
		std::map<uint32_t, std::pair<std::shared_ptr<b2World>, std::shared_ptr<WorldProxy>>> worlds;
		std::map <uint32_t, b2Body*> bodies;

		std::atomic<int> current_pfs;
	
		inline void dowork();
	public:
		int getCurrentPFS() { return current_pfs; }

		friend class WorldProxy;
		Box2DGameComponent(Game &g) : GameComponent(g) { }

		uint32_t getNewBodyID() {
			return sm_body_id.fetch_add(1);
		}

		std::thread::id getThreadID() {
			//log("phys-thread::getThreadID()") << m_worker.get_id() << std::endl;
			return m_worker.get_id();
		}

		void createBody(uint32_t n_world_id, const std::shared_ptr<B2BodyProxy>& body, const b2BodyDef& def) {

			//log("phys::createBody") << "got here!" << std::endl;
			b2Body* w = worlds[body->getBodyID()].first->CreateBody(&def);
			w->SetUserData((void*)body.get());
			bodies[body->getBodyID()] = w;
		}

		void setContactListener(uint32_t n_world_id, b2ContactListener* f) {
			worlds[n_world_id].first->SetContactListener(f);

		}

		void createShape(uint32_t w, uint32_t b, const std::shared_ptr<b2Shape>& def) {
			bodies[b]->CreateFixture(def.get(), 0.0f);
		}

		void createFixture(uint32_t w, const std::shared_ptr<B2BodyProxy>& b, const std::shared_ptr<b2FixtureDef>& def, const std::shared_ptr<b2Shape>& sh) {
			
			auto fixture = bodies[b->getBodyID()]->CreateFixture(def.get());
			if (def->isSensor) {
				fixture->SetSensor(true);
				fixture->SetUserData((void*)FOOT_SENSOR);
			}
		}

		inline void applyForce(const std::shared_ptr<B2BodyProxy>& b, i3d::vec3d f) {
            auto b2_body = bodies[b->getBodyID()];
			b2_body->ApplyForce(to_b2Vec(f), b2_body->GetWorldCenter(), true);
		}

		inline void applyLinearImpulse(const std::shared_ptr<B2BodyProxy>& b, i3d::vec3d f) {
            auto b2_body = bodies[b->getBodyID()];
			b2_body->ApplyLinearImpulse(to_b2Vec(f), b2_body->GetWorldCenter(), true);
		}

		void init() override { m_worker = std::thread{ [this]() { this->dowork(); } }; }

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

		std::shared_ptr<B2BodyProxy> createBody(const b2BodyDef&, std::shared_ptr<B2PhysicsComponent>);
		void createFixture(const std::shared_ptr<B2BodyProxy>&, const std::shared_ptr<b2FixtureDef>&, const std::shared_ptr<b2Shape>&);
		void createShape(const uint32_t b, const std::shared_ptr<b2Shape>& def);
		void receivePFO(std::shared_ptr<PhysicsFrame> pfo);
		void applyForce(const std::shared_ptr<B2BodyProxy>& b, const i3d::vec3d f);
		void applyLinearImpulse(const std::shared_ptr<B2BodyProxy>& b, const i3d::vec3d f);
		void setContactListener(b2ContactListener* f) {
			AsyncExecutor::enqueue(m_master->getThreadID(), [=] { m_master->setContactListener(m_world_id, f); });
		}
		i3d::vec3d getLinearVelocity(const std::shared_ptr<B2BodyProxy>& b) {
			return from_b2Vec(m_master->bodies[b->getBodyID()]->GetLinearVelocity());
		}

		double getMass(const std::shared_ptr<B2BodyProxy>& b) {
			return m_master->bodies[b->getBodyID()]->GetMass();
		}
	};


	

	class B2PhysicsComponent : public EntityComponent, public std::enable_shared_from_this<B2PhysicsComponent> {
		std::shared_ptr<B2BodyProxy> m_body;
		std::shared_ptr<WorldProxy> m_world;
	public:
		B2PhysicsComponent(std::shared_ptr<Entity> parent) : EntityComponent(parent) {
			
		}

		inline std::shared_ptr<WorldProxy> getWorld() { return m_world; }

		inline void setB2Body(const std::shared_ptr<B2BodyProxy>& nb) { m_body = nb; }
		inline const std::shared_ptr<B2BodyProxy>& getBodyID() { return m_body; }

		i3d::vec3d getLinearVelocity() {
			return m_world->getLinearVelocity(m_body);
		}

		double getMass() {
			return m_world->getMass(m_body);
		}

		virtual void registerWithWorld(std::shared_ptr<WorldProxy> world) {	
			m_world = world;
		}

		void applyForce(i3d::vec3d f) {
			m_world->applyForce(m_body, f);
		}

		void applyLinearImpulse(i3d::vec3d f) {
			m_world->applyLinearImpulse(m_body, f);
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
		double m_hw;
		double m_hh;
	public:
		B2PhysicsStatic(std::shared_ptr<Entity> parent, double half_width, double half_height) : B2PhysicsComponent(parent), m_hw(half_width), m_hh(half_height) {}

		void recieveFrame(gecom::PhysicsFrameData pdf) {
		}

		void registerWithWorld(std::shared_ptr<WorldProxy> world) {
			b2BodyDef def;
			def.type = b2_staticBody;
			def.position.Set(getParent()->getPosition().x(), getParent()->getPosition().y());
			def.angle = 0;
			getParent()->setRotation(0);
            auto body = world->createBody(def, shared_from_this());

			auto groundBox = std::make_shared<b2PolygonShape>();
			groundBox->SetAsBox(m_hw, m_hh);

			auto groundFix = std::make_shared<b2FixtureDef>();
			groundFix->shape = groundBox.get();
			groundFix->friction = 1;
			groundFix->density = 0.0;

			world->createFixture(body, groundFix, groundBox);
		}
	};

	void Box2DGameComponent::dowork() {
		auto last_pfs = gecom::really_high_resolution_clock::now();
		int pfs = 0;
		while (true) {
			auto start = gecom::really_high_resolution_clock::now();

			for (auto world : worlds) {
				auto n_pf = std::make_shared<PhysicsFrame>();
				//log("phys-thread") << "# phys bodies: " << world.second.first->GetBodyCount();
				world.second.first->Step(m_time_step, m_velocity_iterations, m_position_iterations);
				auto body = world.second.first->GetBodyList();
				while (body != nullptr) {

                    B2BodyProxy* body_proxy_ptr = (B2BodyProxy*)body->GetUserData();
                    //log("phys-thread") << "body is not nullptr";
					// GetUserData is set when body is constructed
					// It's a void*
					n_pf->add(body_proxy_ptr->getBodyID(), PhysicsFrameData(from_b2Vec(body->GetPosition()), from_b2Vec(body->GetLinearVelocity()), body->GetAngle(), body->GetAngularVelocity()));
					body = body->GetNext();
				}

				auto n_world_proxy = world.second.second;
				AsyncExecutor::enqueueMain([=] { n_world_proxy->receivePFO(n_pf); });
			}

			AsyncExecutor::execute(std::chrono::milliseconds(2));
			pfs++;

			if (gecom::really_high_resolution_clock::now() - last_pfs > std::chrono::seconds(1)) {
				current_pfs.store(pfs);
				pfs = 0;
				last_pfs = gecom::really_high_resolution_clock::now();
			}

			auto dur = gecom::really_high_resolution_clock::now() - start;
			auto sleep_duration = std::chrono::milliseconds(int(m_time_step * 1000.0f)) - dur;
			//log("phys-thread") << "Sleeping for " << sleep_duration.count() << std::endl;
			std::this_thread::sleep_for(sleep_duration);
		}
	}
}

#endif