#ifndef GEGAME_SCENE_HEADER
#define GEGAME_SCENE_HEADER

// temp because fuck it
class vec3 { };

class PhysicsState { 
	vec3 position;
	vec3 velocity;
	float angle;
	float angularVel;
};

class SceneEntity {
	PhysicsState current;
	PhysicsState previous;
};

// An entity controlled via a Box2D b2StaticBody
class StaticEntity : public SceneEntity { };

// An entity controlled via a Box2D b2DynamicBody
class DynamicEntity : public SceneEntity { };

// An entity controlled via a Box2D b2KinematicBody
class KinematicEntity : public SceneEntity { };

// An entity that isn't in the physics simulation
class UnsimulatedEntity : public SceneEntity { };

class Scene {
	std::vector<SceneEntity> entities;

	// add to scene should check if entity is simulated. 
	// Box2D thread could skip simulating scenes that have no simulated objects
	// depends on how we do that whole thing
	// multiple scenes = multiple b2World's, but we only need to actually call
	// b2World::Step() on scenes with > 0 SimulatedEntities
	// idk -> just something to keep in mind whoever gets to write the next code
	// do we need to enforce an entity is only in one scene..?
	// if b2World has everything in the scene inside it, then double simulation 
	// of that body would be a disaster. In fact, I think Box2D blocks it.
	// Either way, something to think about.
};

#endif
