#ifndef GECOM_SCENE_HEADER
#define GECOM_SCENE_HEADER

#include <queue>
#include <algorithm>
#include <type_traits>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include <mutex>
#include <utility>

#include "Entity.hpp"
#include "Initial3D.hpp"
#include "Bound.hpp"
#include "Quadtree.hpp"
#include "Window.hpp"

namespace gecom{

	class Scene;

	// TODO texture unit allocator
	
	// shader program technique
	class Technique {
	private:
		static std::unordered_map<std::type_index, Technique *> sm_singletons;
		static std::mutex sm_mutex;

	public:
		template <typename TechT>
		static inline Technique * singleton() {
			static_assert(std::is_base_of<Technique, TechT>::value, "TechT must be a technique subtype");
			std::lock_guard<std::mutex> lock(sm_mutex);
			auto it = sm_singletons.find(std::type_index(typeid(TechT)));
			if (it == sm_singletons.end()) {
				it = sm_singletons.insert(it, std::make_pair(std::type_index(typeid(TechT)), new TechT()));
			}
			return it->second;
		}

		virtual inline void bind() { }
		virtual inline void unbind() { }
		virtual inline GLuint program() { return 0; }
		virtual inline void update(GLuint prog, const Scene &scene, const i3d::mat4d &mv) { }
		virtual inline ~Technique() { }
	};

	class DefaultTechnique : public Technique {
	private:
		shader_program_spec m_prog_spec;

	public:
		inline DefaultTechnique() {
			m_prog_spec.source("scene_default.glsl");

		}
		virtual inline void bind() {

		}

		virtual inline GLuint program() {
			return Window::currentContext()->shaderManager()->program(m_prog_spec);
		}

		virtual inline void update(GLuint prog, const Scene &scene, const i3d::mat4d &mv) {
			glUniformMatrix4fv(glGetUniformLocation(prog, "modelview_matrix"), 1, true, i3d::mat4f(mv));
			glUniformMatrix4fv(glGetUniformLocation(prog, "projection_matrix"), 1, true, i3d::mat4f::scale(0.1));
		}
	};

	struct draw_type {
		enum type {
			standard,
			shadow,
			colorpick,
			bound
		};
	};

	class draw_call {
	private:
		Technique *m_tech = nullptr;
		i3d::mat4d m_mv;
		std::function<void(void)> m_draw;
		GLuint m_prog = 0;

		inline void reset() {
			m_tech = nullptr;
			m_mv = i3d::mat4d();
			m_draw = std::function<void(void)>();
			m_prog = 0;
		}

	public:
		inline draw_call() { }

		inline draw_call(Technique *tech_, const i3d::mat4d &mv_, std::function<void(void)> draw_) :
			m_tech(tech_), m_mv(mv_), m_draw(draw_), m_prog(tech_->program()) { }

		inline draw_call(const draw_call &other) :
			m_tech(other.m_tech), m_mv(other.m_mv), m_draw(other.m_draw), m_prog(other.m_prog) { }

		inline draw_call(draw_call &&other) :
			m_tech(other.m_tech), m_mv(other.m_mv), m_draw(std::move(other.m_draw)), m_prog(other.m_prog)
		{
			other.reset();
		}

		inline draw_call & operator=(const draw_call &other) {
			m_tech = other.m_tech;
			m_mv = other.m_mv;
			m_draw = other.m_draw;
			m_prog = other.m_prog;
			return *this;
		}

		inline draw_call & operator=(draw_call &&other) {
			m_tech = other.m_tech;
			m_mv = other.m_mv;
			m_draw = std::move(other.m_draw);
			m_prog = other.m_prog;
			other.reset();
			return *this;
		}

		inline operator bool() const {
			return m_tech && m_draw;
		}

		inline bool operator<(const draw_call &rhs) const {
			bool t1 = m_tech < rhs.m_tech;
			bool t2 = m_tech == rhs.m_tech && m_prog < rhs.m_prog;
			return t1 || t2;
		}

		inline Technique * technique() const {
			return m_tech;
		}

		inline GLuint program() const {
			return m_prog;
		}

		inline const i3d::mat4d & modelView() const {
			return m_mv;
		}

		inline void draw() const {
			m_draw();
		}
	};

	class draw_queue {
	private:
		Scene *m_scene = nullptr;
		std::priority_queue<draw_call> m_draw_calls;
		
	public:
		inline draw_queue() { }

		inline draw_queue(Scene *scene_) : m_scene(scene_) { }

		inline draw_queue(const draw_queue &other) : m_draw_calls(other.m_draw_calls), m_scene(other.m_scene) { }

		inline draw_queue(draw_queue &&other) : m_draw_calls(std::move(other.m_draw_calls)), m_scene(other.m_scene) { }

		inline draw_queue & operator=(const draw_queue &other) {
			m_draw_calls = other.m_draw_calls;
			m_scene = other.m_scene;
			return *this;
		}

		inline draw_queue & operator=(draw_queue &&other) {
			m_draw_calls = std::move(other.m_draw_calls);
			m_scene = other.m_scene;
			return *this;
		}

		inline void push(const draw_call &dc) {
			if (dc) {
				m_draw_calls.push(dc);
			}
		}

		inline void execute() const {
			Technique * tech = nullptr;
			GLuint prog = 0;
			glUseProgram(0);

			auto q = m_draw_calls;
	
			while (!q.empty()) {
				draw_call d = q.top();
				q.pop();
				auto tech2 = d.technique();
				if (tech2 != tech) {
					if (tech) tech->unbind();
					tech = tech2;
					auto prog2 = d.program();
					if (prog2 != prog) {
						prog = prog2;
						glUseProgram(prog);
					}
					tech->bind();
				}
				// update
				tech->update(prog, *m_scene, d.modelView());
				// draw
				d.draw();
			}
			if (tech) tech->unbind();
	
			glUseProgram(0);

		}
	};

	class DrawableComponent : public EntityComponent {
	public:
		DrawableComponent(std::shared_ptr<Entity> parent) : EntityComponent(parent) { }

		virtual void pushDrawCalls(draw_queue &q, unsigned dt) { }

		virtual void draw() =0;

		virtual ~DrawableComponent() { }
	};

	class Scene {
	public:
		virtual void add(std::shared_ptr<gecom::Entity>) = 0;
		virtual void addStatic(std::shared_ptr<gecom::Entity> i_entity) { add(i_entity); }
		virtual std::vector<std::shared_ptr<gecom::Entity>>& get_all() = 0;
		virtual ~Scene() { }
	};

	class Camera {
	private:
		std::shared_ptr<Entity> m_entity;
	public:
		Camera(const std::shared_ptr<Entity> &e) : m_entity(e) { }

		const std::shared_ptr<Entity> getEntity() const {
			return m_entity;
		}

		void setEntity(const std::shared_ptr<Entity> &e) {
			m_entity = e;
		}

		virtual ~Camera() { }
	};

	// delete me once sexy quadtree implementation is done
	class InefficentScene : public Scene {
		std::vector<std::shared_ptr<gecom::Entity>> m_entities;

	public:
		void add(std::shared_ptr<gecom::Entity> ne) {
			m_entities.push_back(ne);
		}

		std::vector<std::shared_ptr<gecom::Entity>>& get_all() {
			return m_entities;
		}
	};


	class Camera2D : public Camera {

	public:
		// ???
		double radius() const {
			return 20;
		}
	};

	// this has the sexy quadtree instead of m_entities
	class Scene2D : public Scene {
	private:
		std::vector<std::shared_ptr<gecom::Entity>> m_dynamicEntities;
		quadtree<std::shared_ptr<gecom::Entity>> m_staticEntities;

	public:
		void add(std::shared_ptr<gecom::Entity> ne) {
			m_dynamicEntities.push_back(ne);
		}

		void addStatic(std::shared_ptr<gecom::Entity> i_entity) {
			m_staticEntities.insert(i_entity, i_entity->getWorldAABB());
		}

		std::vector<std::shared_ptr<gecom::Entity>>& get_all() {
			std::vector<std::shared_ptr<gecom::Entity>> allEntities(m_dynamicEntities);
			auto staticEntites = m_staticEntities.search(aabbd(i3d::vec3d(), i3d::vec3d::one() * i3d::math::inf<double>()));
			allEntities.insert(allEntities.end(), staticEntites.begin(), staticEntites.end());
			return allEntities;
		}

		std::vector<std::shared_ptr<Entity>> search(const aabbd &bb) {
			auto v = m_staticEntities.search(bb);
			v.insert(v.end(), m_dynamicEntities.begin(), m_dynamicEntities.end());
			return v;
		}

		std::vector<std::shared_ptr<Entity>> search(const Camera2D &cam) {
			// ???
			aabbd bb(cam.getEntity()->getPosition(), i3d::vec3d::one() * cam.radius() * 1.044);
			return search(bb);
		}

		draw_queue makeDrawQueue(const aabbd &bb, unsigned dt) {
			// TODO draw 'types'
			draw_queue q(this);
			auto v = search(bb);
			for (auto &e : v) {
				auto vd = e->getComponents<DrawableComponent>();
				for (auto &d : vd) {
					d->pushDrawCalls(q, dt);
				}
			}
			return q;
		}
	};

	class Scene3D : Scene {
		// this would be for future sexy 3d octree
	};
}

#endif