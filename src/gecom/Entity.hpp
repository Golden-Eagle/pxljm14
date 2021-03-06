#ifndef GECOM_ENTITY_HEADER
#define GECOM_ENTITY_HEADER

#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>
#include <map>
#include <typeindex>

#include "Chrono.hpp"
#include "GECom.hpp"
#include "Bound.hpp"

namespace gecom {
	class Entity;
	class Scene;
	class WorldProxy;

	using entity_id_t = uint32_t;
	
	class EntityComponent { 
		std::shared_ptr<Entity> m_parent;
	public:
		EntityComponent(std::shared_ptr<Entity> parent) : m_parent(parent) { }
		const std::shared_ptr<Entity>& getParent() const { return m_parent; }
		virtual void update(gecom::really_high_resolution_clock::duration delta) {

		}
	};

	
	class Entity : public std::enable_shared_from_this<Entity> {
		// TODO physics data interpolation?
		i3d::vec3d m_position;
		i3d::vec3d m_velocity;
		double m_rotation;
		double m_angl_vel;

		Scene* m_scene;

		std::shared_ptr<gecom::WorldProxy> m_world;

		std::map<std::type_index, std::vector<std::shared_ptr<EntityComponent>>> components;

		static std::atomic<entity_id_t> sm_ID;
		entity_id_t m_ID = 0;
		std::vector< std::shared_ptr<Entity>> m_children;

	public:
		Entity(const std::shared_ptr<gecom::WorldProxy>& prxy) : m_world(prxy) { m_ID = Entity::sm_ID.fetch_add(1); }

		entity_id_t getID() const { return m_ID; }

		virtual void init(Scene* scene) { m_scene = scene;  }
		Scene* getScene() { return m_scene; }
		const std::shared_ptr<gecom::WorldProxy>& getWorld() { return m_world; }

		virtual void update(gecom::really_high_resolution_clock::duration delta) {
			for (auto cv : components) {
				for (auto c : cv.second) {
					c->update(delta);
				}
			}
		}

		void addChild(std::shared_ptr<Entity>& i_child) {
			m_children.push_back(i_child);
		}

		const std::vector< std::shared_ptr< Entity > > & getChildren() const {
			return m_children;
		}

		i3d::vec3d getPosition() { return m_position; }
		double getRotation() { return m_rotation; }

		// TODO - NOT THIS
		void setPosition(const i3d::vec3d &p) {
			m_position = p;
		}
		
		// TODO - NOT THIS
		void setRotation(const double r) {
			m_rotation = r;
		}

		virtual aabbd getWorldAABB() {
			return aabbd();
		}

		virtual void recomputeWorldAABB() {

		}

		virtual i3d::mat4d getModelWorldMatrix() {
			return i3d::mat4d::translate(getPosition()) * i3d::mat4d::rotateZ(getRotation());
		}

		template <typename B, typename T>
		void addComponent(const std::shared_ptr<T> comp) {
			static_assert(std::is_base_of<B, T>::value, "T must be a sublcass of B");
			components[std::type_index(typeid(B))].push_back(comp);
		}

		template <typename T>
		std::vector<std::shared_ptr<T>> getComponents() {
			std::type_index et(typeid(T));
			std::vector<std::shared_ptr<T>> rets;
			auto it = components.find(et);
			if(it != components.end()) {		
				for(auto c = it->second.begin(); c != it->second.end(); c++) {
					rets.push_back(std::static_pointer_cast<T>(*c));
				}
			}
			return rets;		
		}
	};
}

#endif