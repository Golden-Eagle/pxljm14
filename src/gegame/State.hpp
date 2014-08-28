#ifndef GEGAME_STATE_HEADER
#define GEGAME_STATE_HEADER

#include "Game.hpp"

namespace gegame {
	class Game;

	class State {
	public:
		virtual void init() =0;

		virtual void pause() =0;
		virtual void resume() =0;

		virtual void update(Game*) =0;
		virtual void draw(Game*) =0;

		virtual ~State() {}
	protected:
		State() { }
	};
}

#endif