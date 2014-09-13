
#include <thread>
#include <chrono>
#include <iostream>

#include <gecom/GECom.hpp>
#include <gecom/Window.hpp>
#include <gecom/Resource.hpp>
#include <gecom/Config.hpp>
#include <gecom/Game.hpp>
#include <gecom/State.hpp>
#include <gecom/Concurrent.hpp>
#include <gecom/Shader.hpp>
#include <gecom/Render.hpp>
#include <gecom/Scene.hpp>
#include <gecom/Entity.hpp>
#include <gecom/Box2D.hpp>
#include <gecom/UnitSquare.hpp>
#include <gecom/Render.hpp>

#include <gecom/Quadtree.hpp>

#include "Level.hpp"

using namespace std;
using namespace gecom;
using namespace i3d;

//
// dropping example state machine code here for starters
//

class PauseState : public State<> {
public:
	virtual action_ptr updateForeground() override {
		return nullAction();
	}
};

class PlayState : public State<std::string> {
	std::shared_ptr<Entity> box;
	std::shared_ptr<Entity> ground;
	std::shared_ptr<WorldProxy> world;
	Scene2D m_scene;
	std::shared_ptr<B2PhysicsComponent> player_phs;
	Game* m_game;

public:
	PlayState(Game* game) : m_game(game) {

		world = game->getGCM().get<Box2DGameComponent>()->addWorld(i3d::vec3d(0.0, -20.0, 0.0));

		box = std::make_shared<Entity>();
		box->setPosition(i3d::vec3d(5, 10, 0));
		box->addComponent<DrawableComponent>(std::make_shared<UnitSquare>(box, 1, 2.5));
		player_phs = std::make_shared<B2PhysicsComponent>(box);
		player_phs->registerWithWorld(world);
		box->addComponent<B2PhysicsComponent>(player_phs);

		shared_ptr<Camera> camera(make_shared<Camera>(box));
		m_scene.setCamera(camera);

		m_scene.add(box);

		ground = std::make_shared<Entity>();
		ground->setPosition(i3d::vec3d(0, -4, 0));
		box->addComponent<DrawableComponent>(std::make_shared<UnitSquare>(ground, 5, 1));
		auto gphs = std::make_shared<B2PhysicsStatic>(ground, 5, 1);
		gphs->registerWithWorld(world);
		ground->addComponent<B2PhysicsComponent>(gphs);

		m_scene.add(ground);

		pxljm::LevelGenerator lg;
		auto level = lg.getTestLevel();
		level->load(m_scene, world);
	}

	virtual action_ptr updateForeground() override {
		if(Window::currentContext()->pollKey(GLFW_KEY_ESCAPE)) {
			return callAction<PauseState>();
		}

		// WTF?!
		// auto e = std::make_shared<Entity>();
		// e->addComponent<DrawableComponent>(std::make_shared<SquareDrawableComponent>());
		// myScene.add(e);

		// some kind of call to game->render(myScene);
		//DrawQueue dq;
		//dq.insert(make_shared<DrawCall>(0, e->getComponents<DrawableComponent>()[0], i3d::mat4d()));
		//dq.execute();

		if (Window::currentContext()->pollKey(GLFW_KEY_UP)) {
			player_phs->applyLinearImpulse(i3d::vec3d(0, 100000, 0));
		}
		
		if (Window::currentContext()->getKey(GLFW_KEY_RIGHT)) {
			player_phs->applyLinearImpulse(i3d::vec3d(500, 0, 0));
		}
		
		if (Window::currentContext()->getKey(GLFW_KEY_LEFT)) {
			player_phs->applyLinearImpulse(i3d::vec3d(-500, 0, 0));
		}

		return nullAction();
	}

	virtual void drawForeground() override {
		//log("Test") << "drawing";

		glClear(GL_COLOR_BUFFER_BIT);

		draw_queue q = m_scene.makeDrawQueue(aabbd(vec3d(), vec3d::one() * 20), draw_type::standard);
		q.execute();

	}
};

class LoadingGameState : public State < > {
	Game* m_game;
public:
	LoadingGameState(Game* game) : m_game(game) { }

	virtual action_ptr updateForeground() override {
		return callAction<PlayState>(m_game);
	}
};

class MainMenuState : public State<> {
	Game* m_game;
public:
	MainMenuState(Game* game) : m_game(game) { }
	
	virtual action_ptr updateForeground() override { 
		return callAction<LoadingGameState>(m_game);
	}

};

class StartupState : public State<> {
	Game* m_game;
public:
	StartupState(Game* game) : m_game(game) { }

	virtual action_ptr updateForeground() override {
		return callAction<MainMenuState>(m_game);
	}
};

int main() {
	AsyncExecutor::start();

	// create main context, so it can be shared with
	Window *win = createWindow();
	win->makeContextCurrent();

	win->shaderManager()->addSourceDirectory("./res/shader");
	
	// create contexts for background threads that share with main context
	Window *win2 = createWindow().share(win);
	Window *win3 = createWindow().share(win);

	AsyncExecutor::enqueueFast([=] {
		win2->makeContextCurrent();
	});

	AsyncExecutor::enqueueSlow([=] {
		win3->makeContextCurrent();
	});

	Game pxljm;
	pxljm.create<Box2DGameComponent>();
	pxljm.init<StartupState>(win);
	pxljm.run();

	AsyncExecutor::stop();

	delete win3;
	delete win2;
	delete win;

	glfwTerminate();

	cin.get();

}
