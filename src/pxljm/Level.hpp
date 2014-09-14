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
		enum type{
			dirt = 0,
			grass = 1,
			none = 15
		};

		bool solid;
		bool inf;
		Tile::type tileType;
		Tile() : solid(false), inf(false), tileType(Tile::type::none){  }
		inline bool isEmpty() const { return !solid && !inf && tileType == Tile::type::none; }
	};

	struct EntityPlaceholder {
		enum type{
			enemy,
			collectable,
			tree,
			none
		};
		EntityPlaceholder::type entityType;
		i3d::vec3d position;
		EntityPlaceholder(EntityPlaceholder::type i_type = type::none) : entityType(i_type) {  }
	};

	using tile_column = std::vector<Tile>;
	using tile_grid = std::vector<tile_column>;

	class Chunk : public gecom::Entity {
	public:
		Chunk(const std::shared_ptr<gecom::WorldProxy>& proxy, int i_xpos, int i_ypos, tile_grid i_grid);
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

			gecom::log("LEVEL") << int(img.data()[0]) << " " << int(img.data()[1]) << " " << int(img.data()[2]) << " " << int(img.data()[3]);

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
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			//glBlendEquation(GL_MIN);
			//glBlendFunc(GL_ONE, GL_ONE);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		virtual inline void unbind() {
			glDisable(GL_BLEND);
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
		void addChunk(std::shared_ptr<Chunk> i_chunk);
		void addEntities(const std::vector<EntityPlaceholder> &i_entities);
		void load(gecom::Scene& scene, const std::shared_ptr<gecom::WorldProxy> world);
		void unload(gecom::Scene &scene);
	private:
		std::vector<std::shared_ptr<Chunk>> m_chunks;
		std::vector<EntityPlaceholder> m_entities;
	};

	class LevelGenerator {
	public:
		LevelGenerator();
		int getChunkSize();
		void setChunkSize(int i_size);
		std::shared_ptr<Level> getTestLevel(const std::shared_ptr<gecom::WorldProxy>& world);
		std::shared_ptr<Level>  getLevel(/*something*/);

	private:
		int m_chunkSize;

		std::shared_ptr<Level> compileLevel(const std::shared_ptr<gecom::WorldProxy>& world, tile_grid i_tiles, std::vector<EntityPlaceholder> &i_entities);


		struct BuildingHint {
			double deltaHeight = 0.0;
			double deltaVariance = 0.0;
			double smoothness = 0.5;
			double platformChance = 0.0;
			double dangerChance = 0.0;
			double pickupChance = 0.0;
			BuildingHint() {  };
		};

		enum SpacingHint {
			uniform,
			swing,
			random
		};

		std::vector<int> getSpacing(int i_width, SpacingHint i_spaceHint, int i_avgSize);

		int movingSubpart(int i_startHeight, int i_maxHeight, int i_start, int i_end, tile_grid &io_grid, std::vector<EntityPlaceholder> &io_entities, BuildingHint i_hint);
		int jumpSubpart(int i_startHeight, int i_maxHeight, int i_start, int i_end, tile_grid &io_grid, std::vector<EntityPlaceholder> &io_entities, BuildingHint i_hint);




		//Tile grid helper method
		inline tile_grid LevelGenerator::makeTileGrid(int i_width, int i_height){
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

#endif
