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
	gecom::Config config_test("res/config/PlatformGame.json");
	// int jump_key = config.get("keybindings").get("jump");
	int jump_key = config_test.get("keybingings/jump");
	// int jump_key = config.get("keybingings.jump");


	ResourceManager rm("res/");
	rm.add_cache_file<Text>("example.txt");

	while(rm.cache()) ;

	std::shared_ptr<Text> text_file = rm.get<Text>("example.txt");
	std::cout << text_file->data() << std::endl;

	gegame::Game platformGame;
	platformGame.run(new InitState);

	// Test
	gegame::createWindow().size(1024, 768).title("Golden Eagle").visible(true);

	while(true) ;
}