#ifndef PXLJM_PE_HEADER
#define PXLJM_PE_HEADER

#include <gecom/Entity.hpp>
#include <gecom/Scene.hpp>
#include <gecom/Box2D.hpp>
#include <gecom/Chrono.hpp>
#include <gecom/Window.hpp>
#include <gecom/Scene.hpp>

class PlayerEntity : public gecom::Entity {
	std::shared_ptr<gecom::B2PhysicsComponent> player_phs;
	std::shared_ptr<gecom::WorldProxy> m_world;
	std::shared_ptr<gecom::ProtagonistDrawable> player_dw;
	bool is_running = false;
public:
	PlayerEntity(std::shared_ptr<gecom::WorldProxy>& proxy) : gecom::Entity(), m_world(proxy) { }

	void init(gecom::Scene& s) override {
		gecom::log("player") << "Init()";
		setPosition(i3d::vec3d(5, 50, 0));

		player_dw = std::make_shared<gecom::ProtagonistDrawable>(shared_from_this());
		addComponent<gecom::DrawableComponent>(player_dw);
		player_phs = std::make_shared<gecom::B2PhysicsComponent>(shared_from_this());
		player_phs->registerWithWorld(m_world);
		addComponent<gecom::B2PhysicsComponent>(player_phs);
	}

	void update(gecom::really_high_resolution_clock::duration delta) override {
		Entity::update(delta);

		if (gecom::Window::currentContext()->pollKey(GLFW_KEY_UP)) {
			player_phs->applyLinearImpulse(i3d::vec3d(0, 10000, 0));
			player_dw->startJumpAnimation();
		}

		bool should_be_running = false;// std::abs(player_phs->getLinearVelocity().x()) > 0.0;

		if (gecom::Window::currentContext()->getKey(GLFW_KEY_RIGHT)) {
			player_phs->applyLinearImpulse(i3d::vec3d(100, 0, 0));
			should_be_running = true;
		}
		if (gecom::Window::currentContext()->getKey(GLFW_KEY_LEFT)) {
			player_phs->applyLinearImpulse(i3d::vec3d(-100, 0, 0));
			should_be_running = true;
		}

		if (should_be_running && !is_running) {
			// start run animation
			player_dw->startRunAnimation();
			is_running = true;
		} else if (!should_be_running && is_running) {
			player_dw->stopRunAnimation();
			is_running = false;
		}
	}
};

#endif