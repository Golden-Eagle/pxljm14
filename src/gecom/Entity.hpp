#ifndef GECOM_ENTITY_HEADER
#define GECOM_ENTITY_HEADER

#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

namespace gecom {
	class EntityComponent { };
	class DrawableComponent : public EntityComponent {
	public:
		virtual void draw() =0;
	};

	class Entity {
		i3d::vec3 m_position;
		i3d::vec3 m_velocity;
		i3d::vec3 m_rotation;
		i3d::vec3 m_angl_vel;

		std::map<std::type_index, std::vector<std::shared_ptr<EntityComponent>>> components;

		using entity_id_t = uint32_t;
	private:
		static std::atomic< entity_id_t > sm_ID(1);
		entity_id_t m_ID = 0;
		std::vector< std::shared_ptr< Entity > > m_children;
	public:
		Entity() : m_ID(sm_ID.fetch_add(1)) { }

		entity_id_t getID() { return m_ID; }
		void addChild( std::shared_ptr< Entity > i_child ) {
			m_children.push_back(std::weak_ptr< Entity >(i_child));
		}
		const std::vector< std::shared_ptr< Entity > > & getChildren() {
			return m_children;
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