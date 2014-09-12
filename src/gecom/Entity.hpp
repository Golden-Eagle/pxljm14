#ifndef GECOM_ENTITY_HEADER
#define GECOM_ENTITY_HEADER

#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>
#include <map>
#include <typeindex>

#include "GECom.hpp"
#include "Bound.hpp"

namespace gecom {
	class Entity;

	using entity_id_t = uint32_t;
	
	class EntityComponent { 
		std::shared_ptr<Entity> m_parent;
	public:
		EntityComponent(std::shared_ptr<Entity> parent) : m_parent(parent) { }
		std::shared_ptr<Entity>& getParent() { return m_parent; }
	};
	class DrawableComponent : public EntityComponent {
	public:
		DrawableComponent(std::shared_ptr<Entity> parent) : EntityComponent(parent) { }
		virtual void draw() =0;
	};

	class Entity {
		i3d::vec3d m_position;
		i3d::vec3d m_velocity;
		i3d::vec3d m_rotation;
		i3d::vec3d m_angl_vel;

		std::map<std::type_index, std::vector<std::shared_ptr<EntityComponent>>> components;

		static std::atomic<entity_id_t> sm_ID;
		entity_id_t m_ID = 0;
		std::vector< std::shared_ptr<Entity>> m_children;
	public:
		Entity() { m_ID = Entity::sm_ID.fetch_add(1); }

		entity_id_t getID() const { return m_ID; }
		void addChild(std::shared_ptr<Entity>& i_child) {
			m_children.push_back(i_child);
		}
		const std::vector< std::shared_ptr< Entity > > & getChildren() const {
			return m_children;
		}

		i3d::vec3d getPosition() { return m_position; }

		virtual aabbd getWorldAABB() {
			return aabbd();
		}

		virtual void recomputeWorldAABB() {

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