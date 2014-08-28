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

};

#endif
