
#include <thread>
#include <chrono>
#include <iostream>

#include <gecom/Window.hpp>
#include <gecom/Resource.hpp>
#include <gecom/Config.hpp>
#include <gecom/Game.hpp>
#include <gecom/State.hpp>
#include <gecom/Concurrent.hpp>
#include <gecom/Shader.hpp>

using namespace gecom;
using namespace std;

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



int main() {

	AsyncExecutor::start();

	Window *win = createWindow();
	win->makeContextCurrent();

	win->shaderManager()->addSourceDirectory("./res/shader");

	auto spec = shader_program_spec().source("showtex.glsl").define("MY_MACRO", 3);

	win->shaderManager()->program(spec);
	win->shaderManager()->program(spec);

	// make an error
	try {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 9001);
	} catch (gl_error &e) {
		log() << "DID I JUST CATCH A GL ERROR?";
	}

	Event<int> e1;

	// test auto-cancel on subscription destruction
	{
		auto sub1 = e1.subscribe([](int a) {
			log() << "Event happened: " << a;
			return false;
		});
		e1.notify(42);
		e1.notify(43);
	}
	// you wont see this
	e1.notify(44);

	// test safe cancel after event destruction
	{
		// when this goes out of scope nothing breaks
		subscription sub2;
		{
			Event<int> e2;
			sub2 = e2.subscribe([](int a) {
				log() << "Other event happened: " << a;
				return false;
			});
			e2.notify(9001);
			e2.notify(9002);
		}
	}


	Game platform_game;
	platform_game.init<StartupState>(42);
	platform_game.run();

	AsyncExecutor::stop();
}