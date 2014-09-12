
#include <algorithm>
#include <memory>
#include <vector>


#include <pxljm/Level.hpp>


using namespace std;
using namespace gecom;

namespace pxljm {

	ChunkDrawableComponent::ChunkDrawableComponent(shared_ptr<Chunk> i_chunk) : DrawableComponent(i_chunk){

	}




	//TODO
	Chunk::Chunk(int i_xpos, int i_ypos, LevelGenerator::tile_grid i_grid) : gecom::Entity() {
		// e->addComponent<B2PhysicsComponent>(std::make_shared<B2PhysicsComponent>(e));

		//iterate over the grid and create bounds for the grid
	}



	Level::Level() : m_chunks() {  }

	void Level::addChunk(shared_ptr<Chunk> i_chunk){
		m_chunks.push_back(i_chunk);
	}



	LevelGenerator::LevelGenerator() : m_chunkSize(16) {  }

	int LevelGenerator::getChunkSize() {
		return m_chunkSize;
	}

	void LevelGenerator::setChunkSize(int i_size) {
		m_chunkSize = std::max(i_size, 1);
	}

	std::shared_ptr<Level> LevelGenerator::getTestLevel() {
		int height = 32;
		int width = 32;

		tile_grid grid = makeTileGrid(width, height);


		for (int x = 0; x < width; x++){
			grid[x][0].solid = true;
		}


		//add platforms

		//compile platforms into level...

		//to compile for now
		return std::shared_ptr<Level>(new Level());
	}

	std::shared_ptr<Level> LevelGenerator::getLevel() {
		//perform complex calculations to get a list of components
	}

	std::shared_ptr<Level> LevelGenerator::compileLevel(tile_grid i_tiles)
	{
		shared_ptr<Level> level(new Level());

		int gridXSize = i_tiles.size();
		int gridYSize = i_tiles[0].size();

		int chunkXSize = gridXSize / m_chunkSize;
		int chunkYSize = gridYSize / m_chunkSize;

		for (int cx = 0; cx < chunkXSize; ++cx) {
			for (int cy = 0; cy < chunkYSize; ++cy) {
				int startX = cx * m_chunkSize;
				int startY = cy * m_chunkSize;

				int endX = min((cx + 1) * m_chunkSize, gridXSize);
				int endY = min((cy + 1) * m_chunkSize, gridYSize);

				tile_grid chunkGrid;
				for (int x = startX; x < endX; ++x) {
					tile_column i_col = i_tiles[x];
					tile_column col(i_col.begin() + startY, i_col.begin() + endY);
					chunkGrid.push_back(col);
				}

				//create chunk
				shared_ptr<Chunk> chunk = make_shared<Chunk>(startX, startY, chunkGrid);
				chunk->addComponent<DrawableComponent>(make_shared<ChunkDrawableComponent>(chunk));
				level->addChunk(chunk);
			}
		}
		return shared_ptr<Level>(new Level());
	}
}

