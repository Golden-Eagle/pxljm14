
#include <algorithm>
#include <memory>
#include <random>
#include <vector>


#include <pxljm/Level.hpp>


using namespace std;
using namespace gecom;

namespace pxljm {

	ChunkDrawableComponent::ChunkDrawableComponent(shared_ptr<Chunk> i_chunk) : DrawableComponent(i_chunk), m_instances(0){
		//generate tile
		glGenVertexArrays(1, &m_vaoID);
		glBindVertexArray(m_vaoID);
		glGenBuffers(1, &m_vbo_v); //vertex information
		glGenBuffers(1, &m_vbo_t); //tile information

		//for all tiles (for now)
		float pos[] = {
			0, 0, 0, 0,
			0, 1, 0, 0,
			1, 0, 0, 0,
			1, 1, 0, 0
		};

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_v);
		glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(float), pos, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, nullptr);

		vector<GLuint> tileTypeArray;
		unsigned x = 0;
		for (const tile_column &col : i_chunk->getTileGrid()) {
			unsigned y = 0;
			for (Tile tile : col) {
				if (!tile.isEmpty()) {
					tileTypeArray.push_back(x);
					tileTypeArray.push_back(y);
					tileTypeArray.push_back(0);
					tileTypeArray.push_back(0);

					++m_instances;
				}
				++y;
			}
			++x;
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_t);
		glBufferData(GL_ARRAY_BUFFER, tileTypeArray.size() * sizeof(GLuint), &tileTypeArray[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribIPointer(1, 4, GL_UNSIGNED_INT, 0, nullptr);

		glVertexAttribDivisor(1,1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void ChunkDrawableComponent::draw()
	{
		//log() << m_instances;
		glBindVertexArray(m_vaoID);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_instances);
		glBindVertexArray(0);
	}

	void ChunkDrawableComponent::pushDrawCalls(draw_queue &q, unsigned dt) {
		i3d::vec3d p = getParent()->getPosition();
		switch (dt)
		{
		case draw_type::standard:
			q.push(draw_call(Technique::singleton<ChunkStandardTechnique>(), i3d::mat4d::translate(p), [=] {
				draw();
			}));
			break;
		default:
			break;
		}
	}


	//TODO
	Chunk::Chunk(int i_xpos, int i_ypos, tile_grid i_grid) : gecom::Entity(), m_tileGrid(i_grid) {
		setPosition(i3d::vec3d(double(i_xpos), double(i_ypos), 0.0));
		// e->addComponent<B2PhysicsComponent>(std::make_shared<B2PhysicsComponent>(e));

		//iterate over the grid and create bounds for the grid

		//fillGrid and compute bounding everything
		//TODO

		//0-0-32-32
		//addComponent(make_shared<B2PhysicsStatic>());
		/*int x = 0;
		for (tile_column col : m_tileGrid) {
			for (int y = 0; y < col.size(); ++y) {
				addComponent(make_shared<B2PhysicsStatic>());
			}
			++x;
		}*/


	}

	const tile_grid & Chunk::getTileGrid(){
		return m_tileGrid;
	}



	Level::Level() : m_chunks() {  }

	void Level::addChunk(shared_ptr<Chunk> i_chunk){
		m_chunks.push_back(i_chunk);
	}

	void Level::load(gecom::Scene &scene, std::shared_ptr<gecom::WorldProxy> world){
		log("level") << "Performing level load into scence";
		for (shared_ptr<Chunk> c : m_chunks) {
			for (auto comp : c->getComponents<B2ChunkPhysicsComponent>())
				comp->registerWithWorld(world);

			scene.add(c);
		}
	}

	void Level::unload(Scene &scene){

	}

	LevelGenerator::LevelGenerator() : m_chunkSize(16) {  }

	int LevelGenerator::getChunkSize() {
		return m_chunkSize;
	}

	void LevelGenerator::setChunkSize(int i_size) {
		m_chunkSize = std::max(i_size, 1);
	}

	shared_ptr<Level> LevelGenerator::getTestLevel() {
		int height = 32;
		int width = 32;

		tile_grid grid = makeTileGrid(width, height);

		std::default_random_engine generator;
		std::normal_distribution<double> distribution(-2.0, 2.0);
		int heightNoise = 0;

		int columnHeight = 5;
		for (int x = 0; x < width; x++){
			heightNoise = int(distribution(generator));
			columnHeight = max(min(heightNoise + columnHeight, height), 1);
			for (int y = 0; y < columnHeight; y++){
				grid[x][y].solid = true; //WHY NOT!!!
			}
		}


		//add platforms

		//compile platforms into level...

		//to compile for now
		//return std::shared_ptr<Level>(new Level());
		return compileLevel(grid);
	}

	std::shared_ptr<Level> LevelGenerator::getLevel() {
		////TODO get heigh/width from somewhere
		//int height = 32;
		//int width = 128;

		//tile_grid grid = makeTileGrid(width, height);

		//vector<PlatformComponent> components(genComponents());

		//for (PlatformComponent plat : components) {
		//	plat.
		//}




		//return compileLevel(grid);
		return nullptr;
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
				if (!emptyGrid(chunkGrid)){
					shared_ptr<Chunk> chunk = make_shared<Chunk>(startX, startY, chunkGrid);
					chunk->addComponent<DrawableComponent>(make_shared<ChunkDrawableComponent>(chunk));
					chunk->addComponent<B2ChunkPhysicsComponent>(make_shared<B2ChunkPhysicsComponent>(chunk));
					level->addChunk(chunk);
				}
			}
		}
		return level;
	}
}

