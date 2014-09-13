
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

#include "Level.hpp";

using namespace std;
using namespace gecom;
using namespace i3d;

//
// dropping example state machine code here for starters
//

class PauseState : public State<> {
public:
	virtual action_ptr updateForeground() override {
		log("Pause").warning() << "pause is a lie, exceptioning";
		throw std::runtime_error("fooled you!");
	}
};

class PlayState : public State<std::string> {
public:
	virtual void onInit() override {
		log("Play") << "NOW PLAYING";
	}
	
	virtual action_ptr updateForeground() override {
		std::cout << "[p to pause] enter an int greater than 100:" << std::endl;
		int a;
		std::cin >> a;
		if (!std::cin.fail()) {
			if (a > 100) {
				return returnAction("WOOHOO!");
			} else {
				std::cout << "OH NOES!" << std::endl;
			}
		} else if (!std::cin.bad()) {
			// input not an int
			std::cin.clear();
			std::string s;
			std::cin >> s;
			if (s == "p") {
				return callback(callAction<PauseState>(),
					[](int r) {
						log("Play") << "pause returned normally";
						return nullAction();
					},
					[this](const rethrow_t &rethrow) {
						try {
							rethrow();
						} catch (std::exception &e) {
							log("Play").error() << "pause exceptioned, returning 'error'";
							return returnAction("error");
						}
						return returnAction("");
					}
				);
			}
			throw std::runtime_error("bad input: " + s);
		}
		std::cin.clear();
		return nullAction();
	}
};

class MainMenuState : public State<> {
public:
	MainMenuState(int a, const std::string &b) {
		// the ctor gets the arguments of callAction()
	}
	
	virtual action_ptr updateForeground() override {
		std::cout << "-- Main Menu --" << std::endl;
		std::cout << "1) Play" << std::endl;
		std::cout << "2) Exit" << std::endl;
		int a;
		std::cin >> a;
		if (!std::cin.fail()) {
			if (a == 1) return callback(callAction<PlayState>(),
				[](const std::string &r) {
					log("MainMenu") << "Play returned " << r;
					if (r == "error") {
						log("MainMenu") << "Play returned 'error', exceptioning";
						throw std::runtime_error("thrown by a callback from an action returned by an exception handler");
					}
					return nullAction();
				},
				[](const rethrow_t &rethrow) {
					log("MainMenu").warning() << "I CANT HANDLE THIS!" << std::endl;
					return nullAction();
					// note - this will not stop main menu from being popped
				}
			);
			if (a == 2) return returnAction(0);
		}
		std::cin.clear();
		return nullAction();
	}

};

class StartupState : public State<> {
public:
	StartupState(int a) {

	}

	virtual action_ptr updateForeground() override {
		return callback(callAction<MainMenuState>(9001, "hello world"),
			[this](const int &r) {
				log("Startup") << "MainMenu returned " << r;
				return returnAction(0);
				// now, think about what happens if you return a callAction() from a callback...
			},
			[this](const rethrow_t &rethrow) {
				try {
					rethrow();
				} catch (std::exception &e) {
					log("Startup") << "BUT I CAN!: exception: " << e.what();
				}
				return returnAction(1);
			}
		);
	}
};

class TestState : public State<> {
	std::shared_ptr<Entity> box;
	std::shared_ptr<Entity> ground;
	std::shared_ptr<WorldProxy> world;
	Scene2D m_scene;
	std::shared_ptr<B2PhysicsComponent> player_phs;

public:
	TestState(Game *game) {
		// sceneWorld = game.getComponent<Box2DGameComponent>().addWorld(i3d::vec3f(0.0f, -10.0f, 0.0f));
		world = game->getGCM().get<Box2DGameComponent>()->addWorld(i3d::vec3d(0.0, -10.0, 0.0));

		box = std::make_shared<Entity>();
		box->setPosition(i3d::vec3d(5, 10, 0));
		box->addComponent<DrawableComponent>(std::make_shared<UnitSquare>(box));
		player_phs = std::make_shared<B2PhysicsComponent>(box);
		player_phs->registerWithWorld(world);
		box->addComponent<B2PhysicsComponent>(player_phs);

		m_scene.add(box);

		ground = std::make_shared<Entity>();
		ground->setPosition(i3d::vec3d(0, -10, 0));
		box->addComponent<DrawableComponent>(std::make_shared<UnitSquare>(ground));
		auto gphs = std::make_shared<B2PhysicsStatic>(ground);
		gphs->registerWithWorld(world);
		ground->addComponent<B2PhysicsComponent>(gphs);

		m_scene.add(ground);

		pxljm::LevelGenerator lg;
		auto level = lg.getTestLevel();
		level->load(m_scene, world);

		// auto physComp = std::make_shared<Box2DGameComponent>(sceneWorld);
		// e->addComponent<PhysicsComponent>(physComp);
	}
	

	virtual action_ptr updateForeground() override {
		// WTF?!
		// auto e = std::make_shared<Entity>();
		// e->addComponent<DrawableComponent>(std::make_shared<SquareDrawableComponent>());
		// myScene.add(e);

		// some kind of call to game->render(myScene);
		//DrawQueue dq;
		//dq.insert(make_shared<DrawCall>(0, e->getComponents<DrawableComponent>()[0], i3d::mat4d()));
		//dq.execute();

		if (Window::currentContext()->pollKey(GLFW_KEY_UP)) {
			player_phs->applyLinearImpulse(i3d::vec3d(0, 1000, 0));
		}
		else if (Window::currentContext()->getKey(GLFW_KEY_RIGHT)) {
			player_phs->applyForce(i3d::vec3d(200, 0, 0));
		}
		else if (Window::currentContext()->getKey(GLFW_KEY_LEFT)) {
			player_phs->applyForce(i3d::vec3d(-200, 0, 0));
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
	pxljm.init<TestState>(win);
	pxljm.run();

	AsyncExecutor::stop();

	delete win3;
	delete win2;
	delete win;

	glfwTerminate();

	cin.get();

}
