#include "Game.hpp" 

void gecom::Game::run() {
	if (!game_init)
		throw std::runtime_error("Must call game::init() before game::run()");

	int n_frames = 0;
	gecom::really_high_resolution_clock::time_point last_update;

	while (!state_manager.done()) {

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

		n_frames++;
		if (gecom::really_high_resolution_clock::now() - last_update >= std::chrono::seconds(1)) {
			Window::currentContext()->title(std::string("FPS: ") + std::to_string(n_frames) + ", PFS: " + std::to_string(ec_manager.get<Box2DGameComponent>()->getCurrentPFS()));
			last_update = gecom::really_high_resolution_clock::now();
			n_frames = 0;
		}
	}
}