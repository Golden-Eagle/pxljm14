#ifndef GECOM_BOX2D_HEADER 
#define GECOM_BOX2D_HEADER

#include <thread>
#include <chrono>

#include <Box2D/Box2D.h>

#include "Game.hpp"
#include "Box2D.hpp"

namespace gecom {
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

	class B2PhysicsComponent : public EntityComponent {
	public:
		B2PhysicsComponent(std::shared_ptr<Entity> parent) : EntityComponent(parent) { }
		static std::atomic<uint32_t> sm_b2p_id;
		
		void recieveFrame(gecom::PhysicsFrameData pfd) {
			log("phys-test") << pfd.getPos() << std::endl;
		}
	};

	class Box2DGameComponent;

	class PhysicsFrame {
		std::map<uint32_t, PhysicsFrameData> bodies; 

	public:
		void add(uint32_t nid, PhysicsFrameData b) {
			bodies[nid] = b;
		}
	};

	class WorldProxy {
		std::shared_ptr<Box2DGameComponent> m_master;
		uint32_t m_world_id;

	public:
		WorldProxy(const std::shared_ptr<Box2DGameComponent>& m, uint32_t nid) : m_master(m), m_world_id(nid) { }
	};

	class Box2DGameComponent : public GameComponent, public std::enable_shared_from_this<Box2DGameComponent> {
		std::thread m_worker;

		// todo: Expose these are options to Box2D Game Component
		int m_velocity_iterations = 6;
		int m_position_iterations = 6;
		float m_time_step = 1.0f / 60.0f;

		static std::atomic<uint32_t> sm_world_id;
		std::map<uint32_t, std::pair<std::shared_ptr<b2World>, std::shared_ptr<WorldProxy>>> worlds;

		b2Body* groundBody;
		b2Body* boxBody;

		i3d::vec3d from_b2Vec(b2Vec2 b) {
			return i3d::vec3d(b.x, b.y, 0);
		}

		void dowork() {
			auto start = std::chrono::high_resolution_clock::now();
			auto n_pf = std::make_shared<PhysicsFrame>();

			for (auto world : worlds) {
				world.second.first->Step(m_time_step, m_velocity_iterations, m_position_iterations);
				auto body = world.second.first->GetBodyList();
				while (body != nullptr) {
					// GetUserData is set when body is constructed
					// It's a void*
					n_pf->add((uint32_t)body->GetUserData(), PhysicsFrameData(from_b2Vec(body->GetPosition()), from_b2Vec(body->GetLinearVelocity()), body->GetAngle(), body->GetAngularVelocity()));
					body = body->GetNext();
				}
			}

			// construct PFO and atomic::store it to the main thread


			auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start);
			auto sleep_duration = std::chrono::milliseconds((int)(m_time_step * 1000.0f)) - dur;
			std::this_thread::sleep_for(sleep_duration);
		}

	public:
		friend class WorldProxy;
		Box2DGameComponent(Game &g) : GameComponent(g) { }

		void init() {
			// m_worker = std::thread{ [this]() { this->dowork(); } };

			// test crap
			// b2BodyDef groundBodyDef;
			// groundBodyDef.position.Set(0.0f, -10.0f);
			// groundBody = m_b2_world.CreateBody(&groundBodyDef);

			// b2PolygonShape groundBox;
			// groundBox.SetAsBox(50.0, 10.0f);
			// groundBody->CreateFixture(&groundBox, 0.0f);

			// // a box to drop on the other box
			// b2BodyDef bodyDef;
			// bodyDef.type = b2_dynamicBody;
			// bodyDef.position.Set(0.0f, 4.0f);
			// boxBody = m_b2_world.CreateBody(&bodyDef);

			// b2PolygonShape dynamicBox;
			// dynamicBox.SetAsBox(1.0f, 1.0f);

			// b2FixtureDef fixtureDef;
			// fixtureDef.shape = &dynamicBox;
			// fixtureDef.density = 1.0f;
			// fixtureDef.friction = 0.3f;

			// boxBody->CreateFixture(&fixtureDef);
		}

		inline std::shared_ptr<WorldProxy> addWorld(i3d::vec3d gravity) {
			auto n_world_id = sm_world_id.fetch_add(1);
			auto n_world = std::make_shared<b2World>(b2Vec2(gravity.x(), gravity.y()));
			auto n_world_proxy = std::make_shared<WorldProxy>(shared_from_this(), n_world_id);
			worlds[n_world_id] = std::make_pair(n_world, n_world_proxy);
			return n_world_proxy;			
		}

		void update() {
			// // temporary test crap
			// m_b2_world.Step(m_time_step, m_velocity_iterations, m_position_iterations);
			// std::cout << "Pos: " << boxBody->GetPosition().x << ", " << boxBody->GetPosition().y << std::endl;
			// std::this_thread::sleep_for(std::chrono::milliseconds(16));
		}
	};
};

#endif