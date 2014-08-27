#ifndef STATE_MANAGER_HEADER
#define STATE_MANAGER_HEADER

namespace ge2d {
	class GameState {
	public:
		virtual void init() =0;
		virtual void cleanup() =0;

		virtual void pause() =0;
		virtual void resume() =0;

		virtual bool update(ge2d::Game* game) =0;
		virtual void draw(ge2d::Game* game) =0;

		virtual ~GameState() {}
	protected:
		GameState() { }
	};

	// class MenuState
}

