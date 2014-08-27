#include <vector>

namespace ge2d {
	class Game {
	public:
		Game() { }
		~Game() { }

		void init() {
			// perform engine init
		}

		void push_state(State* state) {
			states.push_back(state);
		}

		void pop_state() {
			states.pop_front();
		}

		void update() {
			states.front()->update();

		}
		void draw() {
			states.front()->draw();
		}

		bool running() { return m_running; }
		void quit() { m_running = false; }

		int run() {
			// TODO Add timing
			init();

			// How to we decide the initial state
			//push_state

			while(running()) {
				update();
				draw();
			}

			cleanup();
			return 0;
		}
	private:
		std::vector<GameState*> states;
		bool m_running;
	};
}