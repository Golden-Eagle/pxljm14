
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

		vector<GLint> tileTypeArray;
		int x = 0;
		for (const tile_column &col : i_chunk->getTileGrid()) {
			int y = 0;
			for (Tile tile : col) {
				if (tile.tileType != Tile::type::none){

					GLint tileTexture = tile.tileType;

					if (tile.inf) {
						for (int i = 0; i < 100; ++i) {
							tileTypeArray.push_back(x);
							tileTypeArray.push_back(y - i);
							tileTypeArray.push_back(tileTexture);
							tileTypeArray.push_back(0);

							++m_instances;
						}
					}
					else {
						tileTypeArray.push_back(x);
						tileTypeArray.push_back(y);
						tileTypeArray.push_back(tileTexture);
						tileTypeArray.push_back(0);

						++m_instances;
					}
				}
				++y;
			}
			++x;
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_t);
		glBufferData(GL_ARRAY_BUFFER, tileTypeArray.size() * sizeof(GLint), &tileTypeArray[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribIPointer(1, 4, GL_INT, 0, nullptr);

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
			q.push(draw_call(Technique::singleton<ChunkStandardTechnique>(), i3d::mat4d::translate(p), [=] (GLuint prog) {
				draw();
			}));
			break;
		default:
			break;
		}
	}



	B2ChunkPhysicsComponent::B2ChunkPhysicsComponent(std::shared_ptr<Chunk> parent) : gecom::B2PhysicsStatic(parent, 0, 0) { }

	void B2ChunkPhysicsComponent::registerWithWorld(std::shared_ptr<gecom::WorldProxy> world) {
		b2Filter ground_filter;

		tile_grid grid = std::static_pointer_cast<Chunk>(getParent())->getTileGrid();
		for (int y = 0; y < grid[0].size(); ++y) {
			for (int x = 0; x < grid.size(); ++x) {

				if (grid[x][y].solid) {
					i3d::vec3d min = getParent()->getPosition() + i3d::vec3d(x, y, 0);
					i3d::vec3d max = min + i3d::vec3d(1, 1, 0);

					bool isInf = grid[x][y].inf;
					if (isInf){
						min.y() -= 100;
					}

					while (x < grid.size()-1 && grid[x+1][y].solid && grid[x+1][y].inf == isInf){
						max.x() += 1;
						++x;
					}

					//make physics
					b2BodyDef nbodydef;
					nbodydef.type = b2_staticBody;
					auto half_size = (max - min) / 2;
					auto body_pos = min + half_size;
					nbodydef.position.Set(body_pos.x(), body_pos.y());
					uint32_t nbody = world->createBody(nbodydef, shared_from_this());

					auto rs = std::make_shared<b2PolygonShape>();
					rs->SetAsBox(half_size.x(), half_size.y());

					auto rf = std::make_shared<b2FixtureDef>();
					rf->filter = ground_filter;
					rf->shape = rs.get();
					rf->friction = 1.0f;
					rf->density = 0.0f;

					world->createFixture(nbody, rf, rs);
				}
			}

		}
	}


	Chunk::Chunk(int i_xpos, int i_ypos, tile_grid i_grid) : gecom::Entity(), m_tileGrid(i_grid) {
		setPosition(i3d::vec3d(double(i_xpos), double(i_ypos), 0.0));
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
		int height = 64;
		int width = 128;

		tile_grid grid = makeTileGrid(width, height);


		auto spaces = getSpacing(width, SpacingHint::uniform, 20);
		std::default_random_engine generator;
		std::uniform_int_distribution<int> typeDistribution(0, 1);

		int colHeight = 3;

		for (int i = 0; i < spaces.size()-1; ++i){
			//int type = typeDistribution(generator);
			int type = 0;
			switch (type){
			case 0:
				colHeight = movingSubpart(colHeight, height, spaces[i], spaces[i + 1], grid, BuildingHint(-0.2, 2.0, 0.0, 0.0));
				break;
			case 1:
				//colHeight = jumpSubpart(colHeight, height, spaces[i], spaces[i + 1], grid, BuildingHint());
				break;
			default:
				//colHeight = movingSubpart(colHeight, height, spaces[i], spaces[i + 1], grid, BuildingHint());
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

  /*int deltaHeight;
	int varience;
	double smoothness;
	double platformChance;*/

	int LevelGenerator::movingSubpart(int i_startHeight, int i_maxHeight, int i_start, int i_end, tile_grid &io_grid, const BuildingHint &i_hint = BuildingHint()){
		//Flat walk

		i_hint.deltaHeight;

		std::default_random_engine generator;
		std::normal_distribution<double> deltaDistribution(i_hint.deltaHeight, i_hint.varience);
		std::uniform_real_distribution<double> platformChance(0, 1);

		vector<int> platformStarts;
		double delta = deltaDistribution(generator);
		double last = i_startHeight;

		//Platform
		if (platformChance(generator) < i_hint.platformChance) {
			for (int x = i_start; x < i_end; ++x) {
				io_grid[x][int(last)].tileType = Tile::type::dirt;
				io_grid[x][int(last)].solid = true;
			}
		}

		//no Platform
		else {
			for (int x = i_start; x < i_end; ++x) {
				last = min(max(last + delta, 0.0), double(i_maxHeight-1));
				io_grid[x][int(last)].tileType = Tile::type::dirt;
				io_grid[x][int(last)].solid = true;
				io_grid[x][int(last)].inf = true;
			}
		}
		return int(last);
	}

	int LevelGenerator::jumpSubpart(int i_startHeight, int i_maxHeight, int i_start, int i_end, tile_grid &io_grid, const BuildingHint &i_hint = BuildingHint()){
		////Flat Jump
		//for (int x = min(i_start + 3, i_end-1); x < i_end; ++x){
		//	for (int y = 0; y < i_height; ++y){
		//		io_grid[x][y].solid = true;
		//	}
		//}
		return i_startHeight;
	}
}

