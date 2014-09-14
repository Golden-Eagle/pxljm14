#include "Player.hpp"

#include <random>

pxljm::JumpContactListener::JumpContactListener(std::shared_ptr<PlayerEntity> e) : m_e(e) {
	gecom::log("jump") << "Listening for shit, bro!";
}

void pxljm::JumpContactListener::BeginContact(b2Contact* contact) {
	//check if fixture A was the foot sensor
	gecom::log("begincontact") << (int)contact->GetFixtureA()->GetUserData() << " " << (int)contact->GetFixtureA()->GetUserData();
	void* fixtureUserData = contact->GetFixtureA()->GetUserData();
	if ((int)fixtureUserData == FOOT_SENSOR)
		m_e->setJumpAvailable(true);

	//check if fixture B was the foot sensor
	fixtureUserData = contact->GetFixtureB()->GetUserData();
	if ((int)fixtureUserData == FOOT_SENSOR)
		m_e->setJumpAvailable(true);
}

void pxljm::JumpContactListener::EndContact(b2Contact* contact) {
	gecom::log("endcontact") << (int)contact->GetFixtureA()->GetUserData() << " " << (int)contact->GetFixtureA()->GetUserData();
	//check if fixture A was the foot sensor
	void* fixtureUserData = contact->GetFixtureA()->GetUserData();
	if ((int)fixtureUserData == FOOT_SENSOR)
		m_e->setJumpAvailable(false);

	//check if fixture B was the foot sensor
	fixtureUserData = contact->GetFixtureB()->GetUserData();
	if ((int)fixtureUserData == FOOT_SENSOR)
		m_e->setJumpAvailable(false);
}

pxljm::ProjectileEntity::ProjectileEntity(std::shared_ptr<gecom::WorldProxy>& prox, i3d::vec3d dir) : gecom::Entity(prox), m_dir(dir) {
	
}

void pxljm::ProjectileEntity::init()  {
	addComponent<gecom::DrawableComponent>(std::make_shared<gecom::UnitSquare>(shared_from_this(), 1, 1));
	auto phys = std::make_shared<ProjectilePhysics>(shared_from_this(), 1, 1, m_dir);
	phys->registerWithWorld(getWorld());
	addComponent<gecom::B2PhysicsComponent>(phys);
}

pxljm::PlayerPhysics::PlayerPhysics(std::shared_ptr<PlayerEntity> parent) : gecom::B2PhysicsComponent(std::static_pointer_cast<gecom::Entity>(parent)) { }

inline void pxljm::PlayerPhysics::registerWithWorld(std::shared_ptr<gecom::WorldProxy> world) {
	B2PhysicsComponent::registerWithWorld(world);

	world->setContactListener(new JumpContactListener(std::static_pointer_cast<PlayerEntity>(getParent())));

	b2BodyDef def;
	def.type = b2_dynamicBody;
	def.fixedRotation = true;
	def.position.Set(getParent()->getPosition().x(), getParent()->getPosition().y());
	def.linearDamping = 0.6f;

	setBodyID(getWorld()->createBody(def, shared_from_this()));

	auto bbb = std::make_shared<b2PolygonShape>();
	bbb->SetAsBox(0.5, 2.2);

	auto fix = std::make_shared<b2FixtureDef>();
	fix->shape = bbb.get();
	fix->density = 130;
	fix->friction = 0.99f;

	auto feet_sensor = std::make_shared<b2PolygonShape>();
	feet_sensor->SetAsBox(0.5, 0.5, b2Vec2(0, -2.2), 0);

	auto feet_sensor_fixture = std::make_shared<b2FixtureDef>();

	feet_sensor_fixture->isSensor = true;
	feet_sensor_fixture->shape = feet_sensor.get();

	//auto bfeet = std::make_shared<b2PolygonShape>();
	//bfeet->SetAsBox(1, 1, b2Vec2(0, -1), 0);
	//
	//auto ffix = std::make_shared<b2FixtureDef>();
	//ffix->shape = bfeet.get();
	//ffix->density = 100;
	//ffix->friction = 0.6;

	world->createFixture(getBodyID(), fix, bbb);
	world->createFixture(getBodyID(), feet_sensor_fixture, feet_sensor);
	//world->createFixture(m_b_id, ffix, bfeet);
}

pxljm::PlayerEntity::PlayerEntity(std::shared_ptr<gecom::WorldProxy>& proxy) : gecom::Entity(proxy) { }

void pxljm::PlayerEntity::init() {
	//gecom::log("player") << "Init()";
	setPosition(i3d::vec3d(5, 50, 0));

	player_dw = std::make_shared<ProtagonistDrawable>(shared_from_this());
	addComponent<gecom::DrawableComponent>(player_dw);
	//addComponent<gecom::DrawableComponent>(std::make_shared<gecom::UnitSquare>(shared_from_this(), 0.5, 2.2));
	player_phs = std::make_shared<PlayerPhysics>(std::static_pointer_cast<PlayerEntity>(shared_from_this()));
	player_phs->registerWithWorld(getWorld());
	addComponent<gecom::B2PhysicsComponent>(player_phs);
}

void pxljm::PlayerEntity::setJumpAvailable(bool should_jump) {
	jump_available = should_jump;

}

void pxljm::PlayerEntity::update(gecom::really_high_resolution_clock::duration delta) {
	Entity::update(delta);

	if (gecom::Window::currentContext()->pollKey(GLFW_KEY_UP) && jump_available) {
		// intent -> registerIntent([=] { player_phs->applyLinearImpulse(..); }, 1000);
		jump_available = false;
		player_phs->applyLinearImpulse(i3d::vec3d(0, 10000, 0));
		player_dw->startJumpAnimation();
	}

	bool should_be_running = false;// std::abs(player_phs->getLinearVelocity().x()) > 0.0;

	if (gecom::Window::currentContext()->pollKey(GLFW_KEY_SPACE)) {
		// create a projectile
		
		i3d::vec3d proj_dir(player_dw->isLeft() ? 1 : -1, 0, 0);
		

		std::normal_distribution<double> distrub(0.0, 1.0);
		auto n_proj = std::make_shared<ProjectileEntity>(getWorld(), proj_dir);
		n_proj->setPosition(getPosition() + i3d::vec3d(distrub(gen), 1, 0) + (proj_dir * 3));
		addChild(std::static_pointer_cast<Entity>(n_proj));
		player_dw->startFireAnimation();
	}

	if (gecom::Window::currentContext()->getKey(GLFW_KEY_RIGHT)) {
		player_phs->applyLinearImpulse(i3d::vec3d(30000.0 * std::chrono::duration_cast<std::chrono::duration<double>>(delta).count(), 0, 0));
		player_dw->setLeft(true);
		should_be_running = true;
	}
	else if (gecom::Window::currentContext()->getKey(GLFW_KEY_LEFT)) {
		player_phs->applyLinearImpulse(i3d::vec3d(-30000.0 * std::chrono::duration_cast<std::chrono::duration<double>>(delta).count(), 0, 0));
		player_dw->setLeft(false);
		should_be_running = true;
	}

	if (should_be_running && !is_running) {
		// start run animation
		player_dw->startRunAnimation();
		is_running = true;
	}
	else if (!should_be_running && is_running) {
		player_dw->stopRunAnimation();
		is_running = false;
	}
}