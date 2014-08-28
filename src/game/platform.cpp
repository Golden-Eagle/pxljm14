#include <gegame/Game.hpp>
#include <gegame/Window.hpp>
#include <gegame/ResourceManager.hpp>

class InitState : public gegame::State {
	void init() { }
	void cleanup() { }
	void pause() { }
	void resume() { }
	void update(gegame::Game* game) { }
	void draw(gegame::Game* game) { }
};

int main() {
	ResourceManager rm("res/");
	rm.add_cache_file<Text>("example.txt");

	while(rm.cache()) ;

	std::shared_ptr<Text> text_file = rm.get<Text>("example.txt");
	std::cout << text_file->data() << std::endl;

	gegame::Game platformGame;
	platformGame.run(new InitState);

	// Test
	gegame::Window *window = gegame::createWindow().size(1024, 768).title("Golden Eagle").visible(true);


	while(true) ;
}