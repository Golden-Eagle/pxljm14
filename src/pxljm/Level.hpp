#include <memory>
#include <vector>

#include <gecom/Entity.hpp>
#include <gecom/Box2D.hpp>
#include <gecom/Bound.hpp>



namespace pxljm {

	struct Tile {
		bool solid;
		Tile() : solid(false) {}
	};

	using tile_column = std::vector<Tile>;
	using tile_grid = std::vector<tile_column>;

	class Chunk : public gecom::Entity {
	public:
		Chunk(int i_xpos, int i_ypos, tile_grid i_grid);
	private:
	};

	class ChunkDrawableComponent : public gecom::DrawableComponent {
	public:
		ChunkDrawableComponent(std::shared_ptr<Chunk> i_chunk);
		virtual void draw() {}
	};

	class ChunkPhysicsComponent : public gecom::EntityComponent{
	public:
	};

	class Level {
	public:
		Level();
		void addChunk(std::shared_ptr<Chunk>);
		void load();
		void unload();
	private:
		std::vector<std::shared_ptr<Chunk>> m_chunks;
	};

	class LevelGenerator {
	public:
		LevelGenerator();
		int getChunkSize();
		void setChunkSize(int i_size);
		std::shared_ptr<Level> getTestLevel();
		std::shared_ptr<Level>  getLevel(/*something*/);

	private:
		int m_chunkSize;

		std::shared_ptr<Level> compileLevel(tile_grid i_tiles);


		//Tile grid helper method
		inline LevelGenerator::tile_grid LevelGenerator::makeTileGrid(int i_width, int i_height){
			tile_grid grid;
			for (int x = 0; x < i_width; x++){
				tile_column col;
				for (int y = 0; y < i_height; y++){
					col.push_back(Tile());
				}
				grid.push_back(col);
			}
			return grid;
		}
	};
}