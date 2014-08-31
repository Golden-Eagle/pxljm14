#ifndef GECOM_GAME_HEADER
#define GECOM_GAME_HEADER

#include "State.hpp"
#include "EngineComponent.hpp"

namespace gecom {
	class Game {
		bool game_init = false;
		StateManager state_manager;
		EngineComponentManager ec_manager;

	public:
		template <typename FirstStateT, typename... ArgTR>
		void init(ArgTR && ...args) {
			game_init = true;
			state_manager.init<FirstStateT>(std::forward<ArgTR>(args)...);

			#ifndef GECOM_NO_DEFAULT_EC
				// add audiomanager ?
				// add resourcemanager ?
				// ?
			#endif
		}

		void run() {
			if(!game_init)
				throw std::runtime_error("Must call game::init() before game::run()");

			// init
			ec_manager.init();

			while(!state_manager.done()) {
				// update
				ec_manager.update();
				state_manager.update();

				// draw
				state_manager.draw();
			}
		}
	};
}

#endif