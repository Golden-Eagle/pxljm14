#ifndef GECOM_BOX2D_HEADER 
#define GECOM_BOX2D_HEADER

#include <thread>
#include <chrono>

#include "Box2D/Box2D.h"

#include "Game.hpp"

namespace gecom {
	class Box2DGameComponent : public GameComponent {
		std::thread m_worker;

		// todo: Expose these are options to Box2D Game Component
		int m_velocity_iterations = 6;
		int m_position_iterations = 6;
		float m_time_step = 1.0f / 60.0f;

		b2World m_b2_world;

		b2Body* groundBody;
		b2Body* boxBody;

		void dowork() {
			auto start = std::chrono::high_resolution_clock::now();
			m_b2_world.Step(m_time_step, m_velocity_iterations, m_position_iterations);
			auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start);
			// auto sleep_duration = m_time_step - dur;
			// std::this_thread::sleep_for(sleep_duration);
		}

	public:
		Box2DGameComponent(Game &g) : GameComponent(g), m_b2_world(b2Vec2(0.0f, -10.0)) { 
			
		}

		void init() {
			// m_worker = std::thread{ [this]() { this->dowork(); } };

			// test crap
			b2BodyDef groundBodyDef;
			groundBodyDef.position.Set(0.0f, -10.0f);
			groundBody = m_b2_world.CreateBody(&groundBodyDef);

			b2PolygonShape groundBox;
			groundBox.SetAsBox(50.0, 10.0f);
			groundBody->CreateFixture(&groundBox, 0.0f);



			// a box to drop on the other box
			b2BodyDef bodyDef;
			bodyDef.type = b2_dynamicBody;
			bodyDef.position.Set(0.0f, 4.0f);
			boxBody = m_b2_world.CreateBody(&bodyDef);

			b2PolygonShape dynamicBox;
			dynamicBox.SetAsBox(1.0f, 1.0f);

			b2FixtureDef fixtureDef;
			fixtureDef.shape = &dynamicBox;
			fixtureDef.density = 1.0f;
			fixtureDef.friction = 0.3f;

			boxBody->CreateFixture(&fixtureDef);
		}

		void update() {
			// temporary test crap
			m_b2_world.Step(m_time_step, m_velocity_iterations, m_position_iterations);
			std::cout << "Pos: " << boxBody->GetPosition().x << ", " << boxBody->GetPosition().y << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
		}
	};
};

#endif