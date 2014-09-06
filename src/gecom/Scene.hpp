#ifndef GECOM_SCENE_HEADER
#define GECOM_SCENE_HEADER

#include "Entity.hpp"

class Scene {
public:
	virtual void add(std::shared_ptr<gecom::Entity>) =0;
};

// delete me once sexy quadtree implementation is done
class InefficentScene : public Scene {
	std::vector<std::shared_ptr<gecom::Entity>> m_entities;

public:
	void add(std::shared_ptr<gecom::Entity> ne) {
		m_entities.push_back(ne);
	}
};

class Scene2D : Scene {
	// this has the sexy quadtree instead of m_entities
};

#endif