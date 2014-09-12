#ifndef GECOM_GAME_HEADER
#define GECOM_GAME_HEADER


#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <typeindex>

#include "Window.hpp"

#include "State.hpp"

namespace gecom {

	class Game;

	class GameComponent {
		// TODO use a pointer for this
		Game& active_game;
	public:
		GameComponent(Game &g) : active_game(g) { }

		virtual void init() =0;
		virtual void update() =0;
	};

	class GameComponentManager {
		std::unordered_map<std::type_index, std::shared_ptr<GameComponent> > GameComponents;

	public:
		// void add(const std::type_index &k, const std::shared_ptr<GameComponent> &nsi) {
		// 	GameComponents[k] = nsi;
		// }

		template <typename T>
		void add(const std::shared_ptr<T> &nsi) {
			static_assert(std::is_base_of<GameComponent, T>::value, "T must implement GameComponent");
			GameComponents[std::type_index(typeid(T))] = nsi;
		}

		template <typename T>
		std::shared_ptr<T> get() {
			auto it = GameComponents.find(std::type_index(typeid(T)));
			if(it != GameComponents.end())
				return std::static_pointer_cast<T>(it->second);
			throw std::runtime_error("No component of that type available");
		}

		template <typename T>
		bool exists() {
			return GameComponents.count(std::type_index(typeid(T))) > 0;
		}

		void init() {
			for(auto it : GameComponents) {
				it.second->init();
			}
		}

		void update() {
			for(auto it : GameComponents) {
				it.second->update();
			}
		}
	};

	class Game {
		bool game_init = false;
		StateManager state_manager;
		GameComponentManager ec_manager;
		Window* win = nullptr;

	public:
		template <typename FirstStateT>
		void init(Window *win_) {
			win = win_;

			// init
			ec_manager.init();

			state_manager.init<FirstStateT>(this);

			#ifndef GECOM_NO_DEFAULT_EC
				// add audiomanager ?
				// add resourcemanager ?
				// ?
			#endif

			// TODO setup event dispatch ???

			win->visible(true);

			game_init = true;
		}

		void run() {
			if(!game_init)
				throw std::runtime_error("Must call game::init() before game::run()");

			while(!state_manager.done()) {

				// run tasks scheduled for the main thread
				AsyncExecutor::execute(std::chrono::milliseconds(1));

				// dispatch events
				glfwPollEvents();

				// update
				ec_manager.update();
				state_manager.update();

				// draw
				state_manager.draw();
				win->swapBuffers();
			}
		}

		GameComponentManager& getGCM() { return ec_manager; }

		template <typename T>
		void create() {
			ec_manager.add(std::make_shared<T>(*this));
		}
	};
}

#endif