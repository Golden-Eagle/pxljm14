#ifndef GECOM_PROT_HEADER
#define GECOM_PROT_HEADER

#include <memory>
#include <spine/spine.h>
#include <spine/extension.h>

#include "PNG.hpp"
#include "Entity.hpp"

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

	void _spAtlasPage_createTexture(spAtlasPage* self, const char* path){
		gecom::image* texture = new gecom::image(gecom::image::type_png(), path, true);
		
		//if (!texture->loadFromFile(path)) return;
		//texture->setSmooth(true);
		self->rendererObject = texture;
		self->width = texture->width();
		self->height = texture->height();
	}

	void _spAtlasPage_disposeTexture(spAtlasPage* self){
		delete (gecom::image*)self->rendererObject;
	}

	char* _spUtil_readFile(const char* path, int* length){
		return _readFile(path, length);
	}

namespace gecom {

	class SpineDrawable : public DrawableComponent, public std::enable_shared_from_this<SpineDrawable> {
		const int cm_time_scale = 1;
		spSkeleton* m_skeleton;
		spAnimationStateData* m_state_data;
		spAnimationState* m_state;
		spSkeletonBounds* m_bounds;
		float* m_world_vertices;

		GLuint vaoID;
		GLuint vboID;

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

		virtual void pushDrawCalls(draw_queue &q, unsigned dt) {
			auto mat = i3d::mat4d();
			switch (dt)
			{
			case draw_type::standard:
				q.push(draw_call(Technique::singleton<DefaultTechnique>(), mat, [=] {
					draw();
				}));
				break;
			default:
				break;
			}
		}

		SpineDrawable(const std::string& n, const std::shared_ptr<Entity> parent) : DrawableComponent(parent) {
			glGenVertexArrays(1, &vaoID);
			glBindVertexArray(vaoID);
			glGenBuffers(1, &vboID);

			glBindBuffer(GL_ARRAY_BUFFER, vboID);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Load atlas, skeleton, and animations.
			m_world_vertices = new float[1000];
			spAtlas* atlas = spAtlas_createFromFile((std::string("res/spine/") + n + "/" + n + ".atlas").c_str(), 0);
			spSkeletonJson* json = spSkeletonJson_create(atlas);
			json->scale = 0.01f;
			spSkeletonData *skeletonData = spSkeletonJson_readSkeletonDataFile(json, (std::string("res/spine/") + n + "/" + n + ".json").c_str());

			if (!skeletonData) {
				printf("%s\n", json->error);
				exit(0);
			}
			spSkeletonJson_dispose(json);
			m_bounds = spSkeletonBounds_create();

			// Configure mixing.
			m_state_data = spAnimationStateData_create(skeletonData);
			spAnimationStateData_setMixByName(m_state_data, "walk", "jump", 0.2f);
			spAnimationStateData_setMixByName(m_state_data, "jump", "run", 0.2f);

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
			m_skeleton->y = getParent()->getPosition().y() - 2.5;
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

					/*vertices[0].color.r = r;
					vertices[0].color.g = g;
					vertices[0].color.b = b;
					vertices[0].color.a = a;*/
					verts[0] = m_world_vertices[SP_VERTEX_X1];
					verts[1] = m_world_vertices[SP_VERTEX_Y1];
					verts[2] = 0;
					/*vertices[0].texCoords.x = regionAttachment->uvs[SP_VERTEX_X1] * image->width();
					vertices[0].texCoords.y = regionAttachment->uvs[SP_VERTEX_Y1] * image->height();*/

					/*vertices[1].color.r = r;
					vertices[1].color.g = g;
					vertices[1].color.b = b;
					vertices[1].color.a = a;*/
					verts[3] = m_world_vertices[SP_VERTEX_X2];
					verts[4] = m_world_vertices[SP_VERTEX_Y2];
					verts[5] = 0;
					/*vertices[1].texCoords.x = regionAttachment->uvs[VERTEX_X2] * size.x;
					vertices[1].texCoords.y = regionAttachment->uvs[VERTEX_Y2] * size.y;*/

					/*vertices[2].color.r = r;
					vertices[2].color.g = g;
					vertices[2].color.b = b;
					vertices[2].color.a = a;*/
					verts[9] = m_world_vertices[SP_VERTEX_X3];
					verts[10] = m_world_vertices[SP_VERTEX_Y3];
					verts[11] = 0;
					/*vertices[2].texCoords.x = regionAttachment->uvs[VERTEX_X3] * size.x;
					vertices[2].texCoords.y = regionAttachment->uvs[VERTEX_Y3] * size.y;*/

					/*vertices[3].color.r = r;
					vertices[3].color.g = g;
					vertices[3].color.b = b;
					vertices[3].color.a = a;*/
					verts[6] = m_world_vertices[SP_VERTEX_X4];
					verts[7] = m_world_vertices[SP_VERTEX_Y4];
					verts[8] = 0;
					/*vertices[3].texCoords.x = regionAttachment->uvs[VERTEX_X4] * size.x;
					vertices[3].texCoords.y = regionAttachment->uvs[VERTEX_Y4] * size.y;*/

		/*			vertexArray->append(vertices[0]);
					vertexArray->append(vertices[1]);
					vertexArray->append(vertices[2]);
					vertexArray->append(vertices[0]);
					vertexArray->append(vertices[2]);
					vertexArray->append(vertices[3]);*/
					//log("spine") << "doing draw";

					glBindBuffer(GL_ARRAY_BUFFER, vboID);
					glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(float), verts, GL_DYNAMIC_DRAW);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindVertexArray(vaoID);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					glBindVertexArray(0);
				} else if (attachment->type == SP_ATTACHMENT_MESH) {
					log("spine") << "attachment-mesh";
				}
				else if (attachment->type == SP_ATTACHMENT_SKINNED_MESH) {
					log("spine") << "attachment-skinned-mesh";
				}
			}
		}
	};

	class ProtagonistDrawable : public SpineDrawable {
	public:
		ProtagonistDrawable(const std::shared_ptr<Entity> parent) : SpineDrawable(std::string("spineboy"), parent) { }
	};
}

#endif