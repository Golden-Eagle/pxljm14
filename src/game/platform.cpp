#include <gegame/Game.hpp>
#include <gegame/Window.hpp>
#include <gegame/ResourceManager.hpp>
#include <ge_common/Config.hpp>

class InitState : public gegame::State {
	void init() { }
	void cleanup() { }
	void pause() { }
	void resume() { }
	void update(gegame::Game* game) { }
	void draw(gegame::Game* game) { }
};

int main() {
	gegame::Game platformGame;
	platformGame.run(new InitState);
}