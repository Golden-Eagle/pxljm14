
#include <thread>
#include <chrono>
#include <iostream>
#include <stdexcept>

#include <gecom/GECom.hpp>
#include <gecom/Window.hpp>
#include <gecom/Resource.hpp>
#include <gecom/Config.hpp>
#include <gecom/Game.hpp>
#include <gecom/State.hpp>
#include <gecom/Concurrent.hpp>
#include <gecom/Shader.hpp>
#include <gecom/Render.hpp>
#include <gecom/Scene.hpp>
#include <gecom/Entity.hpp>
#include <gecom/Box2D.hpp>
#include <gecom/UnitSquare.hpp>
#include <gecom/Chrono.hpp>
#include <gecom/Render.hpp>

#include <gecom/Quadtree.hpp>

#include "Level.hpp"
#include "Player.hpp"

using namespace std;
using namespace gecom;
using namespace i3d;

namespace pxljm {


	class window_close_error : public std::runtime_error {
	public:
		inline window_close_error() : std::runtime_error("window close requested") { }
	};

	void draw_fullscreen() {
		static GLuint vao = 0;
		if (vao == 0) {
			glGenVertexArrays(1, &vao);
		}
		glBindVertexArray(vao);
		glDrawArrays(GL_POINTS, 0, 1);
		glBindVertexArray(0);
	}

	//
	// dropping example state machine code here for starters
	//

	class PauseState : public State < > {
	public:
		virtual action_ptr updateForeground() override {
			return nullAction();
		}
	};

	class PlayState : public State < std::string > {
		std::shared_ptr<Entity> player;
		std::shared_ptr<WorldProxy> world;
		Scene* m_scene;
		Game* m_game;
		GLuint m_tex_bg;

		gecom::really_high_resolution_clock::time_point last_update;

		gecom::subscription window_sub;

		GLuint m_fbo = 0;
		GLuint m_tex_hdr = 0;
		GLuint m_tex_depth = 0;

		double m_exposure = 1;

		void make_fbo(size2i sz) {
			gecom::log() << "RESIZE " << sz.w << ", " << sz.h;

			if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
			glGenFramebuffers(1, &m_fbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

			if (m_tex_hdr) glDeleteTextures(1, &m_tex_hdr);
			if (m_tex_depth) glDeleteTextures(1, &m_tex_depth);

			glGenTextures(1, &m_tex_hdr);
			glGenTextures(1, &m_tex_depth);

			glActiveTexture(GL_TEXTURE0);

			glBindTexture(GL_TEXTURE_2D, m_tex_hdr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, sz.w, sz.h, 0, GL_RGBA, GL_FLOAT, nullptr);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex_hdr, 0);

			glBindTexture(GL_TEXTURE_2D, m_tex_depth);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, sz.w, sz.h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_tex_depth, 0);

			GLenum bufs[] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, bufs);

		}

	public:
		PlayState(Game* game) : m_game(game) {

			window_sub = Window::currentContext()->onResize.subscribe([=](const gecom::window_size_event &e) {
				if (e.size.w > 0 && e.size.h > 0) {
					make_fbo(e.size);
				}
				return false;
			});

			make_fbo(Window::currentContext()->size());

			m_scene = new Scene2D();
			last_update = gecom::really_high_resolution_clock().now();

			gecom::image img_bg(gecom::image::type_png(), "./res/textures/bg.png", true);

			glGenTextures(1, &m_tex_bg);
			glBindTexture(GL_TEXTURE_2D, m_tex_bg);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img_bg.width(), img_bg.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img_bg.data());
			glGenerateMipmap(GL_TEXTURE_2D);

			world = game->getGCM().get<Box2DGameComponent>()->addWorld(i3d::vec3d(0.0, -20.0, 0.0));

			player = std::make_shared<PlayerEntity>(world);

			m_scene->add(player);
			player->setPosition(player->getPosition() + i3d::vec3d::j() * 64);

			auto drone = std::make_shared<DroneEntity>(world, player);
			m_scene->add(drone);

			//box = std::make_shared<Entity>();
			//box->setPosition(i3d::vec3d(5, 50, 0));
			//box->addComponent<DrawableComponent>(std::make_shared<ProtagonistDrawable>(box));
			//box->addComponent<DrawableComponent>(std::make_shared<UnitSquare>(box, 1, 1.5));
			//player_phs = std::make_shared<B2PhysicsComponent>(box);
			//player_phs->registerWithWorld(world);
			//box->addComponent<B2PhysicsComponent>(player_phs);

			/*auto ground = std::make_shared<Entity>(world);
			ground->setPosition(i3d::vec3d(0, -20, 0));
			auto gphs = std::make_shared<B2PhysicsStatic>(ground, 200, 1);
			gphs->registerWithWorld(world);
			ground->addComponent<B2PhysicsComponent>(gphs);

			m_scene->add(ground);*/

			//shared_ptr<SteadyFocusCamera> cameraEntity(make_shared<SteadyFocusCamera>(player));
			auto cameraEntity = std::make_shared<SteadyFocusCamera>(world, player);
			m_scene->add(cameraEntity);

			auto camera = std::make_shared<Camera>(cameraEntity);
			//shared_ptr<Camera> camera(make_shared<Camera>(cameraEntity));
			m_scene->setCamera(camera);

			pxljm::LevelGenerator lg;
			auto level = lg.getTestLevel(world);
			level->load(*m_scene, world);
		}

		virtual action_ptr updateForeground() override {
			if (Window::currentContext()->pollKey(GLFW_KEY_ESCAPE)) {
				return callAction<PauseState>();
			}

			if (Window::currentContext()->pollKey(GLFW_KEY_EQUAL)) {
				m_exposure *= 1.2;
			}

			if (Window::currentContext()->pollKey(GLFW_KEY_MINUS)) {
				m_exposure /= 1.2;
			}

			if (Window::currentContext()->shouldClose()) {
				throw window_close_error();
			}

			// WTF?!
			// auto e = std::make_shared<Entity>();
			// e->addComponent<DrawableComponent>(std::make_shared<SquareDrawableComponent>());
			// myScene.add(e);

			// some kind of call to game->render(myScene);
			//DrawQueue dq;
			//dq.insert(make_shared<DrawCall>(0, e->getComponents<DrawableComponent>()[0], i3d::mat4d()));
			//dq.execute();

			auto delta = gecom::really_high_resolution_clock().now() - last_update;
			last_update = gecom::really_high_resolution_clock().now();
			m_scene->update(delta);

			return nullAction();
		}

		virtual void drawForeground() override {
			//log("Test") << "drawing";

			size2i sz = Window::currentContext()->size();

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
			glViewport(0, 0, sz.w, sz.h);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);

			static shader_program_spec spec_bg = shader_program_spec().source("background.glsl");
			GLuint prog_bg = Window::currentContext()->shaderManager()->program(spec_bg);
			glUseProgram(prog_bg);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_tex_bg);
			glUniform1f(glGetUniformLocation(prog_bg, "ratio"), sz.ratio());
			glUniform1i(glGetUniformLocation(prog_bg, "sampler_bg"), 0);
			draw_fullscreen();

			draw_queue q = m_scene->makeDrawQueue(aabbd(vec3d(), vec3d::one() * 20), draw_type::standard);
			q.execute(sz);


			glDisable(GL_DEPTH_TEST);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glViewport(0, 0, sz.w, sz.h);

			static shader_program_spec spec_hdr = shader_program_spec().source("hdr.glsl");
			GLuint prog_hdr = Window::currentContext()->shaderManager()->program(spec_hdr);
			glUseProgram(prog_hdr);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_tex_hdr);
			glUniform1f(glGetUniformLocation(prog_hdr, "exposure"), m_exposure);
			glUniform1i(glGetUniformLocation(prog_hdr, "sampler_hdr"), 0);
			draw_fullscreen();

			glFinish();

		}
	};

	class LoadingGameState : public State < > {
		Game* m_game;
	public:
		LoadingGameState(Game* game) : m_game(game) { }

		virtual action_ptr updateForeground() override {
			return callAction<PlayState>(m_game);
		}
	};

	class MainMenuState : public State < > {
		Game* m_game;
	public:
		MainMenuState(Game* game) : m_game(game) { }

		virtual action_ptr updateForeground() override {
			return callAction<LoadingGameState>(m_game);
		}

	};

	class StartupState : public State < > {
		Game* m_game;
	public:
		StartupState(Game* game) : m_game(game) { }

		virtual action_ptr updateForeground() override {
			return callAction<MainMenuState>(m_game);
		}
	};

	

}

int main() {
	AsyncExecutor::start();

	// create main context, so it can be shared with
	Window *win = createWindow();
	win->makeContextCurrent();

	win->shaderManager()->addSourceDirectory("./res/shader");

	// create contexts for background threads that share with main context
	Window *win2 = createWindow().share(win);
	Window *win3 = createWindow().share(win);

	AsyncExecutor::enqueueFast([=] {
		win2->makeContextCurrent();
	});

	AsyncExecutor::enqueueSlow([=] {
		win3->makeContextCurrent();
	});

	Game pxljm_game;
	pxljm_game.create<Box2DGameComponent>();
	pxljm_game.init<pxljm::StartupState>(win);
	pxljm_game.run();

	AsyncExecutor::stop();

	delete win3;
	delete win2;
	delete win;

	glfwTerminate();

	cin.get();

}
