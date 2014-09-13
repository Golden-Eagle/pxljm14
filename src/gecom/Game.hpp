#ifndef GECOM_GAME_HEADER
#define GECOM_GAME_HEADER


#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <typeindex>
#include "Chrono.hpp"
#include "Window.hpp"
#include "Box2D.hpp"
#include "State.hpp"
#include "GameComponent.hpp"

namespace gecom {
	class Game {
		bool game_init = false;
		StateManager state_manager;
		GameComponentManager ec_manager;
		Window* win = nullptr;

	public:
		template <typename FirstStateT>
		void init(Window* win_) {
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

		void run();

		inline GameComponentManager& getGCM() { return ec_manager; }

		template <typename T>
		void create() {
			ec_manager.add(std::make_shared<T>(*this));
		}
	};
}

#endif