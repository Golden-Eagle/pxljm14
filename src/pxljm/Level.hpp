#ifndef GECOM_LEVEL_HEADER
#define GECOM_LEVEL_HEADER

#include <memory>
#include <vector>

#include <gecom/Entity.hpp>
#include <gecom/Box2D.hpp>
#include <gecom/Bound.hpp>
#include <gecom/Scene.hpp>
#include <gecom/Window.hpp>
#include <gecom/PNG.hpp>

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
		B2ChunkPhysicsComponent(std::shared_ptr<Chunk> parent);
		void registerWithWorld(std::shared_ptr<gecom::WorldProxy> world);
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
		GLuint m_tex_atlas;

	public:
		inline ChunkStandardTechnique() {
			m_prog_spec.source("chunk_standard.glsl");

			gecom::image img(gecom::image::type_png(), "./res/textures/atlas.png", false);

			unsigned lod = 0;
			while ((1 << lod) <= img.width() && (1 << lod) <= img.height()) lod++;

			glGenTextures(1, &m_tex_atlas);
			glBindTexture(GL_TEXTURE_2D, m_tex_atlas);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, lod);


			// min/max mipmap level???
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data());
			glGenerateMipmap(GL_TEXTURE_2D);

		}

		virtual inline void bind() {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_tex_atlas);
		}

		virtual inline GLuint program() {
			return gecom::Window::currentContext()->shaderManager()->program(m_prog_spec);
		}

		virtual inline void update(GLuint prog, const gecom::Scene &scene, const i3d::mat4d &mv) {
			glUniformMatrix4fv(glGetUniformLocation(prog, "modelview_matrix"), 1, true, i3d::mat4f(mv));
			glUniformMatrix4fv(glGetUniformLocation(prog, "projection_matrix"), 1, true, i3d::mat4f::scale(0.1));
			glUniform1i(glGetUniformLocation(prog, "sampler_atlas"), 0);

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

	class LevelComponent{
	private:
		int m_start;
		int m_end;
	public:
		LevelComponent(int i_start, int i_end);
		virtual void apply(int i_height) = 0;
	};

	class MovingComponent : public LevelComponent{
	public:
		MovingComponent(int i_start, int i_end);
		virtual int apply(int i_currentHeight, tile_grid i_grid);
	};

	class JumpComponent : public LevelComponent{
	public:
		JumpComponent(int i_start, int i_end);
		virtual int apply(int i_currentHeight, tile_grid i_grid);
	};
}

#endif
