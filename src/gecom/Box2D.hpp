#ifndef GECOM_BOX2D_HEADER 
#define GECOM_BOX2D_HEADER

#include <thread>
#include <chrono>

#include <Box2D/Box2D.h>

#include "Game.hpp"
#include "GameComponent.hpp"
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

	inline i3d::vec3d from_b2Vec(b2Vec2 b) {
		return i3d::vec3d(b.x, b.y, 0);
	}

	inline b2Vec2 to_b2Vec(i3d::vec3d i) {
		return b2Vec2(i.x(), i.y());
	}

	const int FOOT_SENSOR = 1293132;

	class MyContactListener : public b2ContactListener
	{
	public:
		MyContactListener() { 
			log("jump") << "Listening for shit, bro!";

		}
		void BeginContact(b2Contact* contact) {
			//check if fixture A was the foot sensor
			log("begincontact") << (int)contact->GetFixtureA()->GetUserData() << " " << (int)contact->GetFixtureA()->GetUserData();
			void* fixtureUserData = contact->GetFixtureA()->GetUserData();
			if ((int)fixtureUserData == FOOT_SENSOR)
				log("jump") << "YES";

			//check if fixture B was the foot sensor
			fixtureUserData = contact->GetFixtureB()->GetUserData();
			if ((int)fixtureUserData == FOOT_SENSOR)
				log("jump") << "YES";
		}

		void EndContact(b2Contact* contact) {
			log("endcontact") << (int)contact->GetFixtureA()->GetUserData() << " " << (int)contact->GetFixtureA()->GetUserData();
			//check if fixture A was the foot sensor
			void* fixtureUserData = contact->GetFixtureA()->GetUserData();
			if ((int)fixtureUserData == FOOT_SENSOR)
				log("jump") << "NO";

			//check if fixture B was the foot sensor
			fixtureUserData = contact->GetFixtureB()->GetUserData();
			if ((int)fixtureUserData == FOOT_SENSOR)
				log("jump") << "NO";
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
			
			auto fixture = bodies[b]->CreateFixture(def.get());
			if (def->isSensor) {
				fixture->SetSensor(true);
				fixture->SetUserData((void*)FOOT_SENSOR);
			}
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
			n_world->SetContactListener(new MyContactListener);
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

		i3d::vec3d getLinearVelocity(uint32_t b) {
			return from_b2Vec(m_master->bodies[b]->GetLinearVelocity());
		}

		double getMass(uint32_t b) {
			return m_master->bodies[b]->GetMass();
		}
	};


	

	class B2PhysicsComponent : public EntityComponent, public std::enable_shared_from_this<B2PhysicsComponent> {
		uint32_t m_b_id;
		std::shared_ptr<WorldProxy> m_world;
	public:
		B2PhysicsComponent(std::shared_ptr<Entity> parent) : EntityComponent(parent) {
			
		}

		i3d::vec3d getLinearVelocity() {
			return m_world->getLinearVelocity(m_b_id);
		}

		double getMass() {
			return m_world->getMass(m_b_id);
		}

		virtual void registerWithWorld(std::shared_ptr<WorldProxy> world) {
			m_world = world;

			b2BodyDef def;
			def.type = b2_dynamicBody;
			def.fixedRotation = true;
			def.position.Set(getParent()->getPosition().x(), getParent()->getPosition().y());
			def.linearDamping = 0.6f;
			m_b_id = world->createBody(def, shared_from_this());

			auto bbb = std::make_shared<b2PolygonShape>();
			bbb->SetAsBox(1, 1.5);

			auto fix = std::make_shared<b2FixtureDef>();
			fix->shape = bbb.get();
			fix->density = 130;
			fix->friction = 0.99f;

			auto feet_sensor = std::make_shared<b2PolygonShape>();
			feet_sensor->SetAsBox(0.3, 0.2, b2Vec2(0, -1.25), 0);
			
			auto feet_sensor_fixture = std::make_shared<b2FixtureDef>();

			feet_sensor_fixture->isSensor = true;
			feet_sensor_fixture->shape = feet_sensor.get();

			//auto bfeet = std::make_shared<b2PolygonShape>();
			//bfeet->SetAsBox(1, 1, b2Vec2(0, -1), 0);
			//
			//auto ffix = std::make_shared<b2FixtureDef>();
			//ffix->shape = bfeet.get();
			//ffix->density = 100;
			//ffix->friction = 0.6;

			world->createFixture(m_b_id, fix, bbb);
			world->createFixture(m_b_id, feet_sensor_fixture, feet_sensor);
			//world->createFixture(m_b_id, ffix, bfeet);
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
			uint32_t bd = world->createBody(def, shared_from_this());


			auto groundBox = std::make_shared<b2PolygonShape>();
			groundBox->SetAsBox(m_hw, m_hh);
			

			auto groundFix = std::make_shared<b2FixtureDef>();
			groundFix->shape = groundBox.get();
			groundFix->friction = 1;
			groundFix->density = 0.0;

			world->createFixture(bd, groundFix, groundBox);
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
					//log("phys-thread") << "body is not nullptr";
					// GetUserData is set when body is constructed
					// It's a void*
					n_pf->add((uint32_t)body->GetUserData(), PhysicsFrameData(from_b2Vec(body->GetPosition()), from_b2Vec(body->GetLinearVelocity()), body->GetAngle(), body->GetAngularVelocity()));
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
};

#endif