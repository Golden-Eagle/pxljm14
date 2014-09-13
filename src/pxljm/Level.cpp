
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



	B2ChunkPhysicsComponent::B2ChunkPhysicsComponent(std::shared_ptr<Chunk> parent) : gecom::B2PhysicsStatic(parent, 0, 0) { }

	void B2ChunkPhysicsComponent::registerWithWorld(std::shared_ptr<gecom::WorldProxy> world) {

		//METHOD NEEDS SERIOUS OPTIMISATIONS
		tile_grid grid = std::static_pointer_cast<Chunk>(getParent())->getTileGrid();
		for (int x = 0; x < grid.size(); ++x) {
			tile_column col = grid[x];
			auto get_tile = [&](int x, int y) -> Tile {
				// clamp to edge of heightmap
				if (y < 0) y = 0;
				if (y >= col.size()) y = col.size()-1;
				if (x < 0) x = 0;
				if (x >= grid.size()) x = grid.size()-1;
				return grid[x][y];
			};

			for (int y = 0; y < col.size(); ++y) {
				if (grid[x][y].solid) {
					if (!get_tile(x, y + 1).solid ||
						!get_tile(x, y - 1).solid ||
						!get_tile(x + 1, y).solid ||
						!get_tile(x - 1, y).solid){
						//make physics
						b2BodyDef nbodydef;
						nbodydef.type = b2_staticBody;
						auto body_pos = getParent()->getPosition() + i3d::vec3d(x + 0.5, y + 0.5, 0);
						nbodydef.position.Set(body_pos.x(), body_pos.y());
						uint32_t nbody = world->createBody(nbodydef, shared_from_this());

						auto rs = std::make_shared<b2PolygonShape>();
						rs->SetAsBox(0.5, 0.5);

						world->createShape(nbody, rs);
					}
				}

			}
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
		int width = 1024;

		tile_grid grid = makeTileGrid(width, height);


		auto spaces = getSpacing(width, SpacingHint::uniform, 6);
		std::default_random_engine generator;
		std::uniform_int_distribution<int> typeDistribution(0, 1);

		int colHeight = 5;

		for (int i = 0; i < spaces.size()-1; ++i){
			int type = typeDistribution(generator);
			switch (type){
			case 0:
				movingSubpart(colHeight, spaces[i], spaces[i + 1], grid, BuildingHint());
				break;
			case 1:
				jumpSubpart(colHeight, spaces[i], spaces[i + 1], grid, BuildingHint());
				break;
			default:
				movingSubpart(colHeight, spaces[i], spaces[i + 1], grid, BuildingHint());
				break;
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

	vector<int> LevelGenerator::getSpacing(int i_width, SpacingHint i_spaceHint, int i_avgSize){
		vector<int> spaces;
		for (int x = 0; x < i_width; x += i_avgSize){
			spaces.push_back(x);
		}
		return spaces;
	}

	void LevelGenerator::movingSubpart(int i_height, int i_start, int i_end, tile_grid &io_grid, BuildingHint i_hint = BuildingHint()){
		//Flat walk
		for (int x = i_start; x < i_end; ++x){
			for (int y = 0; y < i_height-1; ++y){
				io_grid[x][y].solid = true;
			}
		}
	}

	void LevelGenerator::jumpSubpart(int i_height, int i_start, int i_end, tile_grid &io_grid, BuildingHint i_hint = BuildingHint()){
		//Flat Jump
		for (int x = min(i_start + 3, i_end-1); x < i_end; ++x){
			for (int y = 0; y < i_height; ++y){
				io_grid[x][y].solid = true;
			}
		}
	}
}

