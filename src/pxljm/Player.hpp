#ifndef PXLJM_PEA_HEADER
#define PXLJM_PEA_HEADER

#include <gecom/Chrono.hpp>
#include <gecom/Entity.hpp>
#include <gecom/Scene.hpp>
#include <gecom/Box2D.hpp>
#include <gecom/Chrono.hpp>
#include <gecom/Window.hpp>
#include <gecom/Scene.hpp>
#include <gecom/UnitSquare.hpp>
#include <gecom/SpineDrawable.hpp>
#include <random>

#include "ProtagonistDrawable.hpp"

namespace pxljm {
    class PlayerEntity;

	class JumpContactListener : public b2ContactListener
	{
		std::shared_ptr<PlayerEntity> m_e;
	public:
		JumpContactListener(std::shared_ptr<PlayerEntity> e);
		void BeginContact(b2Contact* contact);
		void EndContact(b2Contact* contact);
	};

	class PlayerPhysics : public gecom::B2PhysicsComponent {
	public:
		PlayerPhysics(std::shared_ptr<PlayerEntity> parent);
		inline virtual void registerWithWorld(std::shared_ptr<gecom::WorldProxy> world);
	};

	class ProjectilePhysics : public gecom::B2PhysicsComponent {
		int m_half_width;
		int m_half_height;
	public:
		ProjectilePhysics(std::shared_ptr<gecom::Entity> parent, int half_width, int half_height)
			: gecom::B2PhysicsComponent(parent), m_half_width(half_width), m_half_height(half_height) {}

		inline virtual void registerWithWorld(std::shared_ptr<gecom::WorldProxy> world) {
			B2PhysicsComponent::registerWithWorld(world);

			b2BodyDef def;
			def.type = b2_dynamicBody;
			def.fixedRotation = true;
			def.position.Set(getParent()->getPosition().x(), getParent()->getPosition().y());
			def.linearDamping = 0.6f;

            setB2Body(world->createBody(def, shared_from_this()));

			auto bbb = std::make_shared<b2PolygonShape>();
			bbb->SetAsBox(m_half_width, m_half_height);

			auto fix = std::make_shared<b2FixtureDef>();
			fix->shape = bbb.get();
			fix->density = 130;
			fix->friction = 0.99f;

			world->createFixture(getBodyID(), fix, bbb);
		}
	};

	class ProjectileDrawable : public gecom::SpineDrawable {
	public:
        ProjectileDrawable(const std::shared_ptr<gecom::Entity> parent) : SpineDrawable(std::string("fireball"), parent, 0.003) { }
	};

	class DronePhysics : public gecom::B2PhysicsComponent {
	public:
		DronePhysics(const std::shared_ptr<gecom::Entity>& parent)
			: gecom::B2PhysicsComponent(parent) {}

		inline virtual void registerWithWorld(std::shared_ptr<gecom::WorldProxy> world) {
			B2PhysicsComponent::registerWithWorld(world);

			b2BodyDef def;
			def.type = b2_dynamicBody;
			def.fixedRotation = true;
			def.position.Set(getParent()->getPosition().x(), getParent()->getPosition().y());
			def.linearDamping = 0.1f;
            def.gravityScale = 0;

            setB2Body(world->createBody(def, shared_from_this()));

			auto bbb = std::make_shared<b2PolygonShape>();
			bbb->SetAsBox(0.001f, 0.001f);

			auto fix = std::make_shared<b2FixtureDef>();
			fix->shape = bbb.get();
			fix->density = 100;
			fix->friction = 0.0f;

			world->createFixture(getBodyID(), fix, bbb);
		}

        void recieveFrame(gecom::PhysicsFrameData pfd) override {
            // TODO pass by reference more often neo
            // TODO - NOT THIS

            getParent()->setPosition(pfd.getPos() + i3d::vec3d(0, 2, 0));
            getParent()->setRotation(pfd.getRot());
        }
	};

	class DroneEntity : public gecom::Entity {
		std::shared_ptr<DronePhysics> phys;
        std::shared_ptr<PlayerEntity> player;
        i3d::vec3d direction;
		int m_health = 100;

	public:
		DroneEntity(std::shared_ptr<gecom::WorldProxy> word, std::shared_ptr<gecom::Entity> pl) : gecom::Entity(word), player(std::static_pointer_cast<pxljm::PlayerEntity>(pl)), direction(0.2f, 0, 0) { }

		void init(gecom::Scene* s) override {
			gecom::Entity::init(s);
			
			setPosition(i3d::vec3d(20, 30, 0));
			addComponent<gecom::DrawableComponent>(std::make_shared<DroneDrawable>(shared_from_this()));

			phys = std::make_shared<DronePhysics>(shared_from_this());
			phys->registerWithWorld(getWorld());

			addComponent<gecom::B2PhysicsComponent>(phys);
		}

        void update(gecom::really_high_resolution_clock::duration delta) override;
	};

	class ProjectileEntity : public gecom::Entity {
		i3d::vec3d m_dir;
		uint32_t half_width;
		uint32_t half_height;
	public:
		ProjectileEntity(std::shared_ptr<gecom::WorldProxy> world, i3d::vec3d dir, uint32_t hx, uint32_t hy) : gecom::Entity(world), m_dir(dir), half_width(hx), half_height(hy) { }

		void init(gecom::Scene* s) override {
			gecom::Entity::init(s);

			addComponent<gecom::DrawableComponent>(std::make_shared<ProjectileDrawable>(shared_from_this()));

			auto phys = std::make_shared<ProjectilePhysics>(shared_from_this(), half_width, half_height);
			phys->registerWithWorld(getWorld());
			
			
			/*gecom::log("proj") << "impulse: " << impulse;*/
			phys->applyLinearImpulse(m_dir);
			addComponent<gecom::B2PhysicsComponent > (phys);
		}
	};

	class PlayerEntity : public gecom::Entity {
		std::shared_ptr<PlayerPhysics> player_phs;
		std::shared_ptr<ProtagonistDrawable> player_dw;
		bool is_running = false;
		bool jump_available = true;
		std::normal_distribution<double> norm_dist;

		int m_health = 100;
	public:
		PlayerEntity(std::shared_ptr<gecom::WorldProxy>& proxy);
		void init(gecom::Scene* s) override;
		void startJumping();
		void finishJumping();
        void startFeetOnGround() {
            jump_available = true;
        }
		void update(gecom::really_high_resolution_clock::duration delta) override;
		void kill();
	};
}

#endif