

#include <memory>
#include <vector>

#include <gecom/Entity.hpp>
#include <gecom/Box2D.hpp>



namespace pxljm {

	class ChunkDrawableComponent : gecom::DrawableComponent {

	};

	class ChunkPhysicsComponent : gecom::EntityComponent{

	};


	class Chunk : gecom::Entity {
		Chunk() : gecom::Entity() {
			//create enttity components and register to me


			// e->addComponent<DrawableComponent>(std::make_shared<UnitSquare>(e));
			// e->addComponent<B2PhysicsComponent>(std::make_shared<B2PhysicsComponent>(e));
		}
	};












	struct Level {
		std::vector<std::shared_ptr<Chunk>> chunks;
		Level() : chunks() {
			//level init
		}
	};


	class LevelGenerator {
	private:

	public:
		LevelGenerator(long i_seed = 0l) { }

		std::shared_ptr<Level> getTestlevel() {
			//create Chunks
			//add platforms

			//compile platforms into level...

			//to compile for now
			return std::shared_ptr<Level>(new Level());
		}
	};


}