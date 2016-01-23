#include "engine.hpp"
#include "scene.hpp"
#include "material.hpp"
#include "components.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "controller.hpp"
#include "physics.hpp"
#include "animation.hpp"
#include "audio.hpp"
#include "module.hpp"
#include "gui.hpp"
#include "image.hpp"
#include "glrenderer/renderdevice.hpp"
#include "game.hpp"
#include "args.hpp"
#include <id/id.hpp>
#include <SDL.h>

void init(Game& game)
{
	game.entities = Entities();
	game.entities.add_system<RenderSystem>(game.resources);
	game.entities.add_system<AnimationSystem>();
	game.entities.add_system<PhysicsSystem>();
	game.entities.add_system<AudioSystem>();
	game.entities.add_system<ModuleSystem>();
	game.entities.get_system<ModuleSystem>().load(Engine::settings["modules"], false);
	game.entities.add_system<ImGuiSystem>(game.engine.window);
	game.entities.get_system<ImGuiSystem>().applyDefaultStyle();
	game.scene = SceneLoader(game.entities);
	game.scene.load(game.scenePath, game.resources);
	Entity cameraEnt = game.entities.get_entity_by_tag("camera");
	ASSERT(cameraEnt.is_alive());
	Camera& camera = cameraEnt.get<Camera>();
	cameraEnt.add<Controller>(camera.position, camera.rotation);
	if (cameraEnt.has<btRigidBody>())
		cameraEnt.get<Controller>().body = &cameraEnt.get<btRigidBody>();
	game.entities.get_system<ModuleSystem>().call($id(INIT), &game);
}

int main(int argc, char* argv[])
{
	Args args(argc, argv);
	Game game;
	Resources& resources = game.resources;
	resources.addPath(args.arg<string>(' ', "data", "../data/"));
	game.engine.init(resources.findPath(args.arg<string>('c', "config", "settings.json")));
	if (Engine::settings["moddir"].is_string())
		resources.addPath(Engine::settings["moddir"].string_value());

	if (argc > 1 && argv[argc-1][0] != '-')
		game.scenePath = argv[argc-1];
	else if (Engine::settings["scene"].is_string())
		game.scenePath = Engine::settings["scene"].string_value();
	init(game);

	bool running = true;
	bool active = false;
	bool screenshot = false;
	bool devtools = args.opt('d', "dev") || Engine::settings["devtools"].bool_value();
	SDL_Event e;
	while (running) {
		BEGIN_CPU_SAMPLE(MainLoop)
		RenderSystem& renderer = game.entities.get_system<RenderSystem>();
		PhysicsSystem& physics = game.entities.get_system<PhysicsSystem>();
		AudioSystem& audio = game.entities.get_system<AudioSystem>();
		AnimationSystem& animation = game.entities.get_system<AnimationSystem>();
		ModuleSystem& modules = game.entities.get_system<ModuleSystem>();
		ImGuiSystem& imgui = game.entities.get_system<ImGuiSystem>();
		Entity cameraEnt = game.entities.get_entity_by_tag("camera");
		Controller& controller = cameraEnt.get<Controller>();
		Camera& camera = cameraEnt.get<Camera>();

		imgui.newFrame();

		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				running = false;
				break;
			}

			if (!active && imgui.processEvent(&e))
				continue;

			if (e.type == SDL_KEYUP) {
				SDL_Keysym keysym = e.key.keysym;

				if (keysym.sym == SDLK_ESCAPE) {
					if (active) {
						active = false;
						game.engine.grabMouse(false);
					} //else running = false;
				}

				if ((keysym.mod == KMOD_LALT || keysym.mod == KMOD_RALT) && keysym.sym == SDLK_RETURN) {
					game.engine.fullscreen(!game.engine.fullscreen());
					renderer.device().resizeRenderTargets();
					continue;
				}
				else if (keysym.mod == KMOD_LCTRL && keysym.sym == SDLK_r) {
					modules.call($id(devtools), $id(RELOAD_SHADERS), &game);
					continue;
				}
				else if (keysym.mod == (KMOD_LCTRL|KMOD_LSHIFT) && keysym.sym == SDLK_r) {
					game.reload = true;
					continue;
				}
				else if (keysym.sym == SDLK_F2) {
					renderer.toggleWireframe();
					continue;
				}
				else if (keysym.sym == SDLK_F3) {
					game.engine.vsync(!game.engine.vsync());
					continue;
				}
				else if (keysym.sym == SDLK_F11) {
					devtools = !devtools;
					continue;
				}
				else if (keysym.sym == SDLK_F12) {
					screenshot = true;
					continue;
				}
			}

			if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
				active = !active;
				game.engine.grabMouse(active);
			}
			else if (e.type == SDL_MOUSEMOTION && active) {
				controller.angles.x += -0.05f * e.motion.yrel;
				controller.angles.y += -0.05f * e.motion.xrel;
			}

			modules.call($id(INPUT), &e);
		}

		if (imgui.usingKeyboard())
			controller.enabled = false;
		controller.update(game.engine.dt);

		// Modules
		START_MEASURE(moduleTimeMs)
		modules.call($id(UPDATE), &game);
		END_MEASURE(moduleTimeMs)

		// Animation
		START_MEASURE(animTimeMs)
		animation.update(game.entities, game.engine.dt);
		END_MEASURE(animTimeMs)

		// Physics
		START_MEASURE(physTimeMs)
		physics.step(game.entities, game.engine.dt);
		END_MEASURE(physTimeMs)

		if (cameraEnt.has<btRigidBody>()) {
			btRigidBody& body = cameraEnt.get<btRigidBody>();
			camera.position = convert(body.getCenterOfMassPosition());
			if (cameraEnt.has<GroundTracker>())
				controller.onGround = cameraEnt.get<GroundTracker>().onGround;
		} else if (controller.enabled) {
			camera.position = controller.position;
		}
		if (controller.enabled)
			camera.rotation = controller.rotation;

		// Audio
		START_MEASURE(audioTimeMs)
		audio.update(game.entities, camera);
		END_MEASURE(audioTimeMs)

		// Graphics
		START_MEASURE(renderTimeMs)
		BEGIN_GPU_SAMPLE(GPURender)
		renderer.render(game.entities, camera);
		END_GPU_SAMPLE()
		END_MEASURE(renderTimeMs)

		if (devtools)
			modules.call($id(devtools), $id(DRAW_DEVTOOLS), &game);

		ImGui::Render();

		if (screenshot) {
			START_MEASURE(screenshotMs)
			// Read
			Image shot(Engine::width(), Engine::height(), 3);
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glReadPixels(0, 0, shot.width, shot.height, GL_RGB, GL_UNSIGNED_BYTE, &shot.data[0]);
			// Fix orientation
			const int lineSize = shot.width * 3;
			unsigned char* line_tmp = new unsigned char[lineSize];
			unsigned char* line_a = &shot.data[0];
			unsigned char* line_b = &shot.data[0] + (lineSize * (shot.height - 1));
			while (line_a < line_b) {
				memcpy(line_tmp, line_a, lineSize);
				memcpy(line_a, line_b, lineSize);
				memcpy(line_b, line_tmp, lineSize);
				line_a += lineSize;
				line_b -= lineSize;
			}
			// Save
			string path = "screenshot_" + std::to_string(Engine::timems()) + ".png";
			bool ret = shot.save(path.c_str());
			END_MEASURE(screenshotMs)
			if (ret) logInfo("Screenshot saved to %s (%.1fms)", path.c_str(), screenshotMs);
			else logError("Screenshot failed!");
			screenshot = false;
		}

		BEGIN_CPU_SAMPLE(swap)
		game.engine.swap();
		END_CPU_SAMPLE()

		game.entities.update();

		modules.call($id(devtools), $id(FRAME_END), &game);

		if (game.reload) {
			modules.call($id(DEINIT), &game);
			renderer.reset(game.entities);
			physics.reset();
			game.scene.reset();
			resources.reset();
			init(game);
			game.reload = false;
		}
		END_CPU_SAMPLE(MainLoop)
	}

	game.entities.get_system<RenderSystem>().reset(game.entities); // TODO: Should not be needed...

	game.entities.remove_system<ImGuiSystem>();
	game.entities.remove_system<ModuleSystem>();
	game.entities.remove_system<AudioSystem>();
	game.entities.remove_system<PhysicsSystem>();
	game.entities.remove_system<AnimationSystem>();
	game.entities.remove_system<RenderSystem>();

	game.engine.deinit();

	return EXIT_SUCCESS;
}
