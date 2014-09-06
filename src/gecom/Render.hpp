#ifndef GECOM_RENDER_HPP
#define GECOM_RENDER_HPP

#include <algorithm>

#include "Entity.hpp"

namespace gecom {

	class DrawCall { 
		GLuint shaderID;
		std::shared_ptr<DrawableComponent> drawable;
		i3d::mat4d transform;

	public:
		DrawCall(GLuint ns, std::shared_ptr<DrawableComponent>& nd, i3d::mat4d& nt) :
			shaderID(ns), drawable(nd), transform(nt) { }

		inline GLuint getShaderID() { return shaderID; }
		inline std::shared_ptr<DrawableComponent>& getDrawable() { return drawable; }
		inline i3d::mat4d& getTransform() { return transform; }
	};

	class DrawQueue { 
		std::vector<std::shared_ptr<DrawCall>> m_drawqueue;

		bool m_draw_queue_dirty = false;

		void sortDrawQueue() { 
			if(m_draw_queue_dirty) {
				std::stable_sort(m_drawqueue.begin(), m_drawqueue.end(), [](std::shared_ptr<DrawCall> a, std::shared_ptr<DrawCall> b) {
					return a->getShaderID() < b->getShaderID();
				});
			}
			m_draw_queue_dirty = false;
		}

	public:
		void insert(const std::shared_ptr<DrawCall> &dc) {
			m_drawqueue.push_back(dc);
			m_draw_queue_dirty = true;
		}

		std::vector<std::shared_ptr<DrawCall>>& get() { 
			if(m_draw_queue_dirty)
				sortDrawQueue(); 
			return m_drawqueue;
		}
	};
}
#endif