#ifndef GEGAME_GAME_HEADER
#define GEGAME_GAME_HEADER

#include <vector>

#include "State.hpp"
#include "ResourceManager.hpp"
#include "Scene.hpp"
#include "ge_common/Config.hpp"

namespace gegame {
	class Game {
		ResourceManager* resources;
		gecom::Config* game_config;

		// To force clang to try and compile Scene
		Scene test_scene;
	public:
		void run(State* initial_state) { 
			game_config = new gecom::Config("config.json"); // Should this be 
															// done without a 
															// hardcode - if so
													 		// how?

			std::string res_prefix = game_config
										->get("system")
										->get_string("resources_directory");

			resources = new ResourceManager(res_prefix);

			// here, we should push the initial state to the stack.
		}
	};
}

#endif