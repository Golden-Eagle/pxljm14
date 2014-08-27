#include <ge2d/Game.hpp>

class InitState : public ge2d::State {
	void init() { }
	void cleanup() { }
	void pause() { }
	void resume() { }
	void update(ge2d::Game* game) { }
	void draw(ge2d::Game* game) { }
};

int main() {
	ge2d::Game platformGame;
	platformGame.run(new InitState);
}