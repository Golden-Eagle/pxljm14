#ifndef PXLJM_PE_HEADER
#define PXLJM_PE_HEADER

#include <gecom/Chrono.hpp>
#include <gecom/Entity.hpp>
#include <gecom/Scene.hpp>
#include <gecom/Box2D.hpp>
#include <gecom/Chrono.hpp>
#include <gecom/Window.hpp>
#include <gecom/Scene.hpp>
#include <random>
#include <gecom/UnitSquare.hpp>

#include "ProtagonistDrawable.hpp"

namespace pxljm {

	class PlayerEntity;

	class ProjectileEntity : public gecom::Entity {
		i3d::vec3d m_dir;
	public:
		ProjectileEntity(std::shared_ptr<gecom::WorldProxy>& prox, i3d::vec3d dir);
		void init() override;
	};

	class JumpContactListener : public b2ContactListener
	{
		std::shared_ptr<PlayerEntity> m_e;
	public:
		JumpContactListener(std::shared_ptr<PlayerEntity> e);
		void BeginContact(b2Contact* contact);
		void EndContact(b2Contact* contact);
	};	

	class ProjectilePhysics : public gecom::B2PhysicsComponent {
		double half_width;
		double half_height;
		i3d::vec3d m_dir;
	public:
		ProjectilePhysics(std::shared_ptr<gecom::Entity> parent, double hx, double hy, i3d::vec3d dir)
			: gecom::B2PhysicsComponent(parent), half_width(hx), half_height(hy), m_dir(dir) { }

		inline virtual void registerWithWorld(std::shared_ptr<gecom::WorldProxy> world) override {
			B2PhysicsComponent::registerWithWorld(world);

			//world->setContactListener(new JumpContactListener(std::static_pointer_cast<PlayerEntity>(getParent())));

			b2BodyDef def;
			def.type = b2_dynamicBody;
			def.fixedRotation = true;
			def.position.Set(getParent()->getPosition().x(), getParent()->getPosition().y());
			def.linearDamping = 0.6f;

			setBodyID(getWorld()->createBody(def, shared_from_this()));
			gecom::log("projectile") << "m_dir: " << m_dir;
			world->applyLinearImpulse(getBodyID(), m_dir * 10000);

			auto bbb = std::make_shared<b2PolygonShape>();
			bbb->SetAsBox(half_width, half_height);

			auto fix = std::make_shared<b2FixtureDef>();
			fix->shape = bbb.get();
			fix->density = 0;
			fix->friction = 0.0;

			//auto feet_sensor = std::make_shared<b2PolygonShape>();
			//feet_sensor->SetAsBox(0.5, 0.5, b2Vec2(0, -2.2), 0);

			//auto feet_sensor_fixture = std::make_shared<b2FixtureDef>();

			//feet_sensor_fixture->isSensor = true;
			//feet_sensor_fixture->shape = feet_sensor.get();

			//auto bfeet = std::make_shared<b2PolygonShape>();
			//bfeet->SetAsBox(1, 1, b2Vec2(0, -1), 0);
			//
			//auto ffix = std::make_shared<b2FixtureDef>();
			//ffix->shape = bfeet.get();
			//ffix->density = 100;
			//ffix->friction = 0.6;

			world->createFixture(getBodyID(), fix, bbb);
			//world->createFixture(getBodyID(), feet_sensor_fixture, feet_sensor);
			//world->createFixture(m_b_id, ffix, bfeet);
		}
	};

	class PlayerPhysics : public gecom::B2PhysicsComponent {
	public:
		PlayerPhysics(std::shared_ptr<PlayerEntity> parent);
		inline virtual void registerWithWorld(std::shared_ptr<gecom::WorldProxy> world) override;
	};



	class PlayerEntity : public gecom::Entity {
		std::mt19937 gen;

		std::shared_ptr<PlayerPhysics> player_phs;

		std::shared_ptr<ProtagonistDrawable> player_dw;
		bool is_running = false;
		bool jump_available = true;
	public:
		PlayerEntity(std::shared_ptr<gecom::WorldProxy>& proxy);

		void init() override;
		void setJumpAvailable(bool should_jump);
		void update(gecom::really_high_resolution_clock::duration delta);
	};
}

#endif