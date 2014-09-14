#ifndef GECOM_PROT_HEADER
#define GECOM_PROT_HEADER

#include <memory>
#include <spine/spine.h>
#include <spine/extension.h>

#include <gecom/PNG.hpp>
#include <gecom/Entity.hpp>
#include <gecom/Scene.hpp>

	//class SkeletonDrawable : public sf::Drawable {
	//public:
	//	Skeleton* skeleton;
	//	AnimationState* state;
	//	float timeScale;
	//	sf::VertexArray* vertexArray;

	//	SkeletonDrawable(SkeletonData* skeleton, AnimationStateData* stateData = 0);
	//	~SkeletonDrawable();

	//	void update(float deltaTime);

	//	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	//private:
	//	bool ownsAnimationStateData;
	//	float* worldVertices;
	//};

	

namespace pxljm {
	class SpineTechnique : public gecom::Technique {
		gecom::shader_program_spec spec;
	public:
		SpineTechnique() {
			spec.source("spine.glsl");
		}

		virtual GLuint program() override {
			return gecom::Window::currentContext()->shaderManager()->program(spec);
		}

		virtual void update(GLuint prog, const gecom::draw_queue &q, const i3d::mat4d &mv, const gecom::size2i &sz) {
			gecom::Technique::update(prog, q, mv, sz);
		}

		inline void bind() {
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		inline void unbind() {
			glDisable(GL_BLEND);
		}
	};

	class SpineDrawable : public gecom::DrawableComponent, public std::enable_shared_from_this<SpineDrawable> {
		const int cm_time_scale = 1.8;
		spSkeleton* m_skeleton;
		spAnimationStateData* m_state_data;
		spAnimationState* m_state;
		spSkeletonBounds* m_bounds;
		float* m_world_vertices;

		GLuint vaoID;
		GLuint vertexVBO;
		GLuint uvVBO;
		GLuint tex_id;

		double m_scale;

	public:
		static void callback(spAnimationState* state, int trackIndex, spEventType type, spEvent* event, int loopCount) {
			spTrackEntry* entry = spAnimationState_getCurrent(state, trackIndex);
			const char* animationName = (entry && entry->animation) ? entry->animation->name : 0;

			switch (type) {
			case SP_ANIMATION_START:
				printf("%d start: %s\n", trackIndex, animationName);
				break;
			case SP_ANIMATION_END:
				printf("%d end: %s\n", trackIndex, animationName);
				break;
			case SP_ANIMATION_COMPLETE:
				printf("%d complete: %s, %d\n", trackIndex, animationName, loopCount);
				break;
			case SP_ANIMATION_EVENT:
				printf("%d event: %s, %s: %d, %f, %s\n", trackIndex, animationName, event->data->name, event->intValue, event->floatValue,
					event->stringValue);
				break;
			}
			fflush(stdout);
		}

		virtual void pushDrawCalls(gecom::draw_queue &q, unsigned dt) {
			auto mat = i3d::mat4d();
			switch (dt)
			{
			case gecom::draw_type::standard:
				q.push(gecom::draw_call(gecom::Technique::singleton<SpineTechnique>(), mat, [=] (GLuint prog) {
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, tex_id);
					glUniform1i(glGetUniformLocation(prog, "sampler_thing"), 0);

					draw();
				}));
				break;
			default:
				break;
			}
		}

		void setLeft(bool is_left) {
			m_skeleton->flipX = !is_left;
		}

		bool isLeft() {
			return !m_skeleton->flipX;
		}

		void startRunAnimation() {
			spAnimationState_setAnimationByName(m_state, 0, "run", true);
		}

		void stopRunAnimation() {
			spAnimationState_setAnimationByName(m_state, 0, "idle", true);
		}

		void startJumpAnimation() {
			spAnimationState_setAnimationByName(m_state, 0, "jump", false);
			spAnimationState_addAnimationByName(m_state, 0, "idle", true, 0);
		}

		void startDeathAnimation() {
			spAnimationState_setAnimationByName(m_state, 0, "die", false);
		}

		SpineDrawable(const std::string& n, const std::shared_ptr<gecom::Entity> parent, double scale) : gecom::DrawableComponent(parent), m_scale(scale) {
			glGenVertexArrays(1, &vaoID);
			glBindVertexArray(vaoID);

			glGenBuffers(1, &vertexVBO);
			glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

			glGenBuffers(1, &uvVBO);
			glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

			glBindBuffer(GL_ARRAY_BUFFER, 0);


			// Load atlas, skeleton, and animations.
			m_world_vertices = new float[1000];

				spAtlas* atlas = spAtlas_createFromFile((std::string("res/spine/") + n + "/" + n + ".atlas").c_str(), 0);
				gecom::image* img = (gecom::image*)(atlas->pages[0].rendererObject);


			glGenTextures(1, &tex_id);
			glBindTexture(GL_TEXTURE_2D, tex_id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


			// min/max mipmap level???
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img->width(), img->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img->data());
			glGenerateMipmap(GL_TEXTURE_2D);


			spSkeletonJson* json = spSkeletonJson_create(atlas);
			json->scale = m_scale;
			spSkeletonData *skeletonData = spSkeletonJson_readSkeletonDataFile(json, (std::string("res/spine/") + n + "/" + n + ".json").c_str());

			if (!skeletonData) {
				printf("%s\n", json->error);
				exit(0);
			}
			spSkeletonJson_dispose(json);
			m_bounds = spSkeletonBounds_create();

			// Configure mixing.
			m_state_data = spAnimationStateData_create(skeletonData);
			spAnimationStateData_setMixByName(m_state_data, "idle", "run", 0.6f);
			spAnimationStateData_setMixByName(m_state_data, "run", "idle", 0.6f);
			spAnimationStateData_setMixByName(m_state_data, "run", "jump", 0.6f);
			spAnimationStateData_setMixByName(m_state_data, "jump", "run", 0.6f);
			spAnimationStateData_setMixByName(m_state_data, "idle", "die", 0.5f);
			spAnimationStateData_setMixByName(m_state_data, "run", "die", 0.5f);
			spAnimationStateData_setMixByName(m_state_data, "die", "run", 1.0f);

			//spSkeletonDrawable* drawable = new spSkeletonDrawable(skeletonData, stateData);

			m_skeleton = spSkeleton_create(skeletonData);
			if (m_state_data == nullptr)
				m_state_data = spAnimationStateData_create(skeletonData);

			m_state = spAnimationState_create(m_state_data);

			spSkeleton_setToSetupPose(m_skeleton);
			m_skeleton->x = getParent()->getPosition().x();
			m_skeleton->y = getParent()->getPosition().y();

			spSkeleton_updateWorldTransform(m_skeleton);

			//spSlot* headSlot = spSkeleton_findSlot(m_skeleton, "head");

			m_state->listener = callback;
			/*if (false) {
				spAnimationState_setAnimationByName(m_state, 0, "test", true);
				}
				else {*/
			spAnimationState_setAnimationByName(m_state, 0, "idle", true);

			//spAnimationState_addAnimationByName(m_state, 0, "jump", false, 3);
			//spAnimationState_addAnimationByName(m_state, 0, "run", true, 0);
		//}


			//sf::RenderWindow window(sf::VideoMode(640, 480), "Spine SFML - spineboy");
			//window.setFramerateLimit(60);
			//sf::Event event;
			//sf::Clock deltaClock;
			//while (window.isOpen()) {
			//	while (window.pollEvent(event))
			//		if (event.type == sf::Event::Closed) window.close();

			//	float delta = deltaClock.getElapsedTime().asSeconds();
			//	deltaClock.restart();

			//	SkeletonBounds_update(bounds, skeleton, true);
			//	sf::Vector2i position = sf::Mouse::getPosition(window);
			//	if (SkeletonBounds_containsPoint(bounds, position.x, position.y)) {
			//		headSlot->g = 0;
			//		headSlot->b = 0;
			//	}
			//	else {
			//		headSlot->g = 1;
			//		headSlot->b = 1;
			//	}

			//	drawable->update(delta);

			//	window.clear();
			//	window.draw(*drawable);
			//	window.display();
			//}

			//SkeletonData_dispose(skeletonData);
			//SkeletonBounds_dispose(bounds);
			//Atlas_dispose(atlas);
		}

		void update(gecom::really_high_resolution_clock::duration delta) {
			double deltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(delta).count();
			//log("spine") << "doing dat update: " << deltaTime;
			m_skeleton->x = getParent()->getPosition().x();
			m_skeleton->y = getParent()->getPosition().y() - 2.2;
			spSkeletonBounds_update(m_bounds, m_skeleton, true);
			spSkeleton_update(m_skeleton, deltaTime);
			spAnimationState_update(m_state, deltaTime * cm_time_scale);
			spAnimationState_apply(m_state, m_skeleton);
			spSkeleton_updateWorldTransform(m_skeleton);
		}

		void draw() {
			i3d::vec3d vertices[4];
			i3d::vec3d vertex;

			

			for (int i = 0; i < m_skeleton->slotsCount; ++i) {
				spSlot* slot = m_skeleton->drawOrder[i];
				spAttachment* attachment = slot->attachment;

				if (!attachment) continue;
				gecom::image* image = nullptr;

				if (attachment->type == SP_ATTACHMENT_REGION) {

					spRegionAttachment* regionAttachment = (spRegionAttachment*)attachment;
					image = (gecom::image*)((spAtlasRegion*)regionAttachment->rendererObject)->page->rendererObject;
					spRegionAttachment_computeWorldVertices(regionAttachment, slot->bone, m_world_vertices);

					uint8_t r = static_cast<uint8_t>(m_skeleton->r * slot->r * 255);
					uint8_t g = static_cast<uint8_t>(m_skeleton->g * slot->g * 255);
					uint8_t b = static_cast<uint8_t>(m_skeleton->b * slot->b * 255);
					uint8_t a = static_cast<uint8_t>(m_skeleton->a * slot->a * 255);

					float verts[12];
					float uvs[8];

					/*vertices[0].color.r = r;
					vertices[0].color.g = g;
					vertices[0].color.b = b;
					vertices[0].color.a = a;*/
					verts[0] = m_world_vertices[SP_VERTEX_X1];
					verts[1] = m_world_vertices[SP_VERTEX_Y1];
					verts[2] = 0;

					uvs[0] = regionAttachment->uvs[SP_VERTEX_X1];
					uvs[1] = regionAttachment->uvs[SP_VERTEX_Y1];

					/*vertices[0].texCoords.x = regionAttachment->uvs[SP_VERTEX_X1] * image->width();
					vertices[0].texCoords.y = regionAttachment->uvs[SP_VERTEX_Y1] * image->height();*/

					/*vertices[1].color.r = r;
					vertices[1].color.g = g;
					vertices[1].color.b = b;
					vertices[1].color.a = a;*/
					verts[3] = m_world_vertices[SP_VERTEX_X2];
					verts[4] = m_world_vertices[SP_VERTEX_Y2];
					verts[5] = 0;

					uvs[2] = regionAttachment->uvs[SP_VERTEX_X2];
					uvs[3] = regionAttachment->uvs[SP_VERTEX_Y2];
					/*vertices[1].texCoords.x = regionAttachment->uvs[VERTEX_X2] * size.x;
					vertices[1].texCoords.y = regionAttachment->uvs[VERTEX_Y2] * size.y;*/

					/*vertices[2].color.r = r;
					vertices[2].color.g = g;
					vertices[2].color.b = b;
					vertices[2].color.a = a;*/
					verts[9] = m_world_vertices[SP_VERTEX_X3];
					verts[10] = m_world_vertices[SP_VERTEX_Y3];
					verts[11] = 0;

					uvs[6] = regionAttachment->uvs[SP_VERTEX_X3];
					uvs[7] = regionAttachment->uvs[SP_VERTEX_Y3];


					/*vertices[2].texCoords.x = regionAttachment->uvs[VERTEX_X3] * size.x;
					vertices[2].texCoords.y = regionAttachment->uvs[VERTEX_Y3] * size.y;*/

					/*vertices[3].color.r = r;
					vertices[3].color.g = g;
					vertices[3].color.b = b;
					vertices[3].color.a = a;*/
					verts[6] = m_world_vertices[SP_VERTEX_X4];
					verts[7] = m_world_vertices[SP_VERTEX_Y4];
					verts[8] = 0;

					uvs[4] = regionAttachment->uvs[SP_VERTEX_X4];
					uvs[5] = regionAttachment->uvs[SP_VERTEX_Y4];
					
					/*vertices[3].texCoords.x = regionAttachment->uvs[VERTEX_X4] * size.x;
					vertices[3].texCoords.y = regionAttachment->uvs[VERTEX_Y4] * size.y;*/

		/*			vertexArray->append(vertices[0]);
					vertexArray->append(vertices[1]);
					vertexArray->append(vertices[2]);
					vertexArray->append(vertices[0]);
					vertexArray->append(vertices[2]);
					vertexArray->append(vertices[3]);*/
					//log("spine") << "doing draw";

					glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
					glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(float), verts, GL_DYNAMIC_DRAW);
					glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
					glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(float), uvs, GL_DYNAMIC_DRAW);

					glBindVertexArray(vaoID);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					glBindVertexArray(0);
				} else if (attachment->type == SP_ATTACHMENT_MESH) {
					gecom::log("spine") << "attachment-mesh";
				}
				else if (attachment->type == SP_ATTACHMENT_SKINNED_MESH) {
					gecom::log("spine") << "attachment-skinned-mesh";
				}
			}
		}
	};

	class ProtagonistDrawable : public SpineDrawable {
	public:
		ProtagonistDrawable(const std::shared_ptr<gecom::Entity> parent) : SpineDrawable(std::string("protagonist"), parent, 0.015) { }
	};

	class DroneDrawable : public SpineDrawable {
	public:
		DroneDrawable(const std::shared_ptr<gecom::Entity> parent) : SpineDrawable(std::string("drone"), parent, 0.005) { }
	};
}

#endif
