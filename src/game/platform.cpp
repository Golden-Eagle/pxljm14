#include <gegame/Game.hpp>
#include <gegame/Window.hpp>

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

	// Test
	gegame::Window *window = gegame::createWindow().size(1024, 768).title("Golden Eagle").visible(true);


	while(true) ;
}