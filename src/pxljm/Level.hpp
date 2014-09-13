#include <memory>
#include <vector>

#include <gecom/Entity.hpp>
#include <gecom/Box2D.hpp>
#include <gecom/Bound.hpp>
#include <gecom/Scene.hpp>
#include <gecom/Window.hpp>

namespace pxljm {

	struct Tile {
		bool solid;
		Tile() : solid(false) {}
		inline bool isEmpty() { return !solid; }
	};

	using tile_column = std::vector<Tile>;
	using tile_grid = std::vector<tile_column>;

	class Chunk : public gecom::Entity {
	public:
		Chunk(int i_xpos, int i_ypos, tile_grid i_grid);
		const tile_grid &getTileGrid();
	private:
		tile_grid m_tileGrid;
	};

	

	class B2ChunkPhysicsComponent : public gecom::B2PhysicsStatic {
	public:
		B2ChunkPhysicsComponent(std::shared_ptr<Chunk> parent) : gecom::B2PhysicsStatic(parent) { }

		void registerWithWorld(std::shared_ptr<gecom::WorldProxy> world) {

			int x = 0;
			for (auto col : std::static_pointer_cast<Chunk>(getParent())->getTileGrid()) {
				int y = 0;
				for (auto tile : col) {
					if (tile.solid) {
						b2BodyDef nbodydef;
						nbodydef.type = b2_staticBody;
						auto body_pos = getParent()->getPosition() + i3d::vec3d(x+0.5, y+0.5, 0);
						gecom::log("chunk-phys") << "Creating B2ChunkPhysicsComponent at " << body_pos.x() << ", " << body_pos.y();
						nbodydef.position.Set(body_pos.x(), body_pos.y());
						uint32_t nbody = world->createBody(nbodydef, shared_from_this());

						auto rs = std::make_shared<b2PolygonShape>();
						rs->SetAsBox(0.5, 0.5);

						world->createShape(nbody, rs);
					}
					y++;
				}
				x++;
			}
		}
	};

	class ChunkDrawableComponent : public gecom::DrawableComponent {
	private:
		int m_instances;
		GLuint m_vaoID;
		GLuint m_vbo_v; //vertex information
		GLuint m_vbo_t; //per tile information
	public:
		ChunkDrawableComponent(std::shared_ptr<Chunk> i_chunk);
		virtual void draw();
		virtual void ChunkDrawableComponent::pushDrawCalls(gecom::draw_queue &q, unsigned dt);
	};

	class ChunkStandardTechnique : public gecom::Technique {
	private:
		gecom::shader_program_spec m_prog_spec;

	public:
		inline ChunkStandardTechnique() {
			m_prog_spec.source("chunk_standard.glsl");

		}
		virtual inline void bind() {

		}

		virtual inline GLuint program() {
			return gecom::Window::currentContext()->shaderManager()->program(m_prog_spec);
		}

		virtual inline void update(GLuint prog, const gecom::Scene &scene, const i3d::mat4d &mv) {
			glUniformMatrix4fv(glGetUniformLocation(prog, "modelview_matrix"), 1, true, i3d::mat4f(mv));
			glUniformMatrix4fv(glGetUniformLocation(prog, "projection_matrix"), 1, true, i3d::mat4f::scale(0.1));
		}
	};

	//class ChunkPhysicsComponent : public gecom::EntityComponent {
	//public:
	//};

	class Level {
	public:
		Level();
		void addChunk(std::shared_ptr<Chunk>);
		void load(gecom::Scene& scene, const std::shared_ptr<gecom::WorldProxy> world);
		void unload(gecom::Scene &scene);
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

		inline bool emptyGrid(tile_grid i_grid){
			for (tile_column col : i_grid) {
				for (Tile tile : col){
					if (!tile.isEmpty())
						return false;
				}
			}
			return true;
		}
	};
}