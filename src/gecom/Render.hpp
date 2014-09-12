#ifndef GECOM_RENDER_HPP
#define GECOM_RENDER_HPP

#include <queue>
#include <algorithm>
#include <type_traits>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include <mutex>
#include <utility>

#include "Window.hpp"
#include "Scene.hpp"
#include "Entity.hpp"

namespace gecom {

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
			auto it = sm_singletons.find(std::type_index(typeid(T)));
			if (it == sm_singletons.end()) {
				it = sm_singletons.insert(it, std::make_pair(std::type_index(typeid(T)), new TechT()));
			}
			return it->second;
		}

		virtual inline void bind() { }
		virtual inline GLuint program() { return 0; }
		virtual inline void update(const Scene &scene, const i3d::mat4d &mv) { }
		virtual inline ~Technique() { }
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
			return m_tech;
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
		std::priority_queue<draw_call> m_draw_calls;
		
	public:
		inline draw_queue() { }

		inline draw_queue(const draw_queue &other) : m_draw_calls(other.m_draw_calls) { }

		inline draw_queue(draw_queue &&other) : m_draw_calls(std::move(other.m_draw_calls)) { }

		inline draw_queue & operator=(const draw_queue &other) {
			m_draw_calls = other.m_draw_calls;
			return *this;
		}

		inline draw_queue & operator=(draw_queue &&other) {
			m_draw_calls = std::move(other.m_draw_calls);
			return *this;
		}

		inline void push(const draw_call &dc) {
			if (dc) {
				m_draw_calls.push(dc);
			}
		}

		inline void execute(const Scene &scene) {
			Technique * tech = nullptr;
			GLuint prog = 0;
			glUseProgram(0);

			auto q = m_draw_calls;
	
			while (!q.empty()) {
				draw_call d = q.top();
				q.pop();
				auto tech2 = d.technique();
				if (tech2 != tech) {
					tech = tech2;
					auto prog2 = d.program();
					if (prog2 != prog) {
						prog = prog2;
						glUseProgram(prog);
					}
					tech->bind();
				}
				// update
				tech->update(scene, d.modelView());
				// draw
				d.draw();
			}
	
			glUseProgram(0);

		}
	};
}
#endif