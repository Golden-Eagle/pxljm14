#include <memory>
#include <vector>

#include <gecom/Entity.hpp>
#include <gecom/Box2D.hpp>
#include <gecom/Bound.hpp>
#include <gecom/Scene.hpp>



namespace pxljm {

	struct Tile {
		bool solid;
		Tile() : solid(true) {}
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

	class ChunkPhysicsComponent : public gecom::EntityComponent {
	public:
	};

	class Level {
	public:
		Level();
		void addChunk(std::shared_ptr<Chunk>);
		void load(gecom::Scene &scene);
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
	};
}