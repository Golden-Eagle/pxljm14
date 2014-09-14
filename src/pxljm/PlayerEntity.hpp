#ifndef PXLJM_PE_HEADER
#define PXLJM_PE_HEADER

#include <gecom/Chrono.hpp>
#include <gecom/Entity.hpp>
#include <gecom/Scene.hpp>
#include <gecom/Box2D.hpp>
#include <gecom/Chrono.hpp>
#include <gecom/Window.hpp>
#include <gecom/Scene.hpp>

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

	class PlayerEntity : public gecom::Entity {
		std::shared_ptr<PlayerPhysics> player_phs;
		std::shared_ptr<gecom::WorldProxy> m_world;
		std::shared_ptr<ProtagonistDrawable> player_dw;
		bool is_running = false;
		bool jump_available = true;
	public:
		PlayerEntity(std::shared_ptr<gecom::WorldProxy>& proxy);

		void init(gecom::Scene& s);
		void setJumpAvailable(bool should_jump);
		void update(gecom::really_high_resolution_clock::duration delta);
	};

}

#endif