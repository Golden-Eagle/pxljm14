#include "PlayerEntity.hpp"

PlayerEntity::PlayerEntity(std::shared_ptr<gecom::WorldProxy>& proxy) : gecom::Entity(), m_world(proxy) { }

void PlayerEntity::init(gecom::Scene& s) {
	gecom::log("player") << "Init()";
	setPosition(i3d::vec3d(5, 50, 0));

	player_dw = std::make_shared<gecom::ProtagonistDrawable>(shared_from_this());
	addComponent<gecom::DrawableComponent>(player_dw);
	player_phs = std::make_shared<PlayerPhysics>(shared_from_this());
	player_phs->registerWithWorld(m_world);
	addComponent<gecom::B2PhysicsComponent>(player_phs);
}

void PlayerEntity::setJumpAvailable(bool should_jump) {
	jump_available = should_jump;
}

void PlayerEntity::update(gecom::really_high_resolution_clock::duration delta) {
	Entity::update(delta);

	if (gecom::Window::currentContext()->pollKey(GLFW_KEY_UP) && jump_available) {
		// intent -> registerIntent([=] { player_phs->applyLinearImpulse(..); }, 1000);

		player_phs->applyLinearImpulse(i3d::vec3d(0, 10000, 0));
		player_dw->startJumpAnimation();
	}

	bool should_be_running = false;// std::abs(player_phs->getLinearVelocity().x()) > 0.0;

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

#include "PlayerPhysics.hpp"

JumpContactListener::JumpContactListener(std::shared_ptr<PlayerEntity> e) : m_e(e) {
	gecom::log("jump") << "Listening for shit, bro!";
}

void JumpContactListener::BeginContact(b2Contact* contact) {
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

void JumpContactListener::EndContact(b2Contact* contact) {
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

PlayerPhysics::PlayerPhysics(std::shared_ptr<PlayerEntity> parent) : gecom::B2PhysicsComponent(std::static_pointer_cast<gecom::Entity>(parent)) { }

inline void PlayerPhysics::registerWithWorld(std::shared_ptr<gecom::WorldProxy> world) {
	B2PhysicsComponent::registerWithWorld(world);

	getWorld()->setContactListener(new JumpContactListener(std::static_pointer_cast<PlayerEntity>(getParent())));

	b2BodyDef def;
	def.type = b2_dynamicBody;
	def.fixedRotation = true;
	def.position.Set(getParent()->getPosition().x(), getParent()->getPosition().y());
	def.linearDamping = 0.6f;

	setBodyID(getWorld()->createBody(def, shared_from_this()));

	auto bbb = std::make_shared<b2PolygonShape>();
	bbb->SetAsBox(1, 1.5);

	auto fix = std::make_shared<b2FixtureDef>();
	fix->shape = bbb.get();
	fix->density = 130;
	fix->friction = 0.99f;

	auto feet_sensor = std::make_shared<b2PolygonShape>();
	feet_sensor->SetAsBox(0.3, 0.2, b2Vec2(0, -1.25), 0);

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

	getWorld()->createFixture(getBodyID(), fix, bbb);
	getWorld()->createFixture(getBodyID(), feet_sensor_fixture, feet_sensor);
	//world->createFixture(m_b_id, ffix, bfeet);
}