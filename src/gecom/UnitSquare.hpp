#ifndef GECOM_UNITSQUARE_HEADER
#define GECOM_UNITSQUARE_HEADER

#include "gecom/GL.hpp"

#include "Entity.hpp"
#include "Scene.hpp"

namespace gecom {
	class UnitSquare : public DrawableComponent {
		GLuint vaoID;
		GLuint vboID;
	public:
		UnitSquare(std::shared_ptr<Entity> parent, float half_width, float half_height) : DrawableComponent(parent) {
			glGenVertexArrays(1, &vaoID);
			glBindVertexArray(vaoID);
			glGenBuffers(1, &vboID);

			float pos[] = {
				-1.0f * half_width, -1.0f * half_height, 0.0f,
				-1.0f * half_width, 1.0f * half_height, 0.0f,
				1.0f * half_width, -1.0f * half_height, 0.0f,
				1.0f * half_width, 1.0f * half_height, 0.0f
			};
			glBindBuffer(GL_ARRAY_BUFFER, vboID);
			glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(float), pos, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void draw() {
			glBindVertexArray(vaoID);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glBindVertexArray(0);
		}

		virtual void pushDrawCalls(draw_queue &q, unsigned dt) {
			auto mat = getParent()->getModelWorldMatrix();
			switch (dt)
			{
			case draw_type::standard : 
				q.push(draw_call(Technique::singleton<DefaultTechnique>(),mat, [=] (GLuint prog) {
					draw();
				}));
				break;
			default:
				break;
			}
		}
	};
}

#endif