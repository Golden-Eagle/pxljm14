#ifndef GECOM_SCENE_HEADER
#define GECOM_SCENE_HEADER

#include "Entity.hpp"
#include "Initial3D.hpp"
#include "Bound.hpp"
#include "Quadtree.hpp"

namespace gecom{

	class Scene {
	public:
		virtual void add(std::shared_ptr<gecom::Entity>) = 0;
		virtual void addStatic(std::shared_ptr<gecom::Entity> i_entity) { add(i_entity); }
		virtual std::vector<std::shared_ptr<gecom::Entity>>& get_all() = 0;
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

	// this has the sexy quadtree instead of m_entities
	class Scene2D : Scene {
	private:
		std::vector<std::shared_ptr<gecom::Entity>> m_dynamicEntities;
		quadtree<std::shared_ptr<gecom::Entity>> m_staticEntities;

	public:
		void add(std::shared_ptr<gecom::Entity> ne) {
			m_dynamicEntities.push_back(ne);
		}

		void addStatic(std::shared_ptr<gecom::Entity> i_entity) {
			//m_staticEntities.insert(i_entity, i_entity->);
		}

		std::vector<std::shared_ptr<gecom::Entity>>& get_all() {
			std::vector<std::shared_ptr<gecom::Entity>> allEntities(m_dynamicEntities);
			auto staticEntites = m_staticEntities.search(aabbd(i3d::vec3d(), i3d::vec3d::one() * i3d::math::inf<double>()));
			allEntities.insert(allEntities.end(), staticEntites.begin(), staticEntites.end());
			return allEntities;
		}
	};

	class Scene3D : Scene {
		// this would be for future sexy 3d octree
	};
}

#endif