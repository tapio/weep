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
#include "triggers.hpp"
#include "gui.hpp"
#include "image.hpp"
#include "glrenderer/renderdevice.hpp"
#include "game.hpp"
#include "args.hpp"
#include <SDL.h>

#if EMBED_MODULES
#define DECLARE_MODULE_FUNC(name) void ModuleFunc_##name(uint msg, void* param)
#define REGISTER_MODULE_FUNC(modules, name) modules.registerEmbeddedModule(#name, ModuleFunc_##name)
DECLARE_MODULE_FUNC(asteroids);
DECLARE_MODULE_FUNC(console);
DECLARE_MODULE_FUNC(devtools);
DECLARE_MODULE_FUNC(fallingsand);
DECLARE_MODULE_FUNC(logo);
DECLARE_MODULE_FUNC(pong);
DECLARE_MODULE_FUNC(settings);
DECLARE_MODULE_FUNC(skyrunner);
DECLARE_MODULE_FUNC(testbed);
#endif

#define GAME_WORLD 0

static Controller s_controllerBackup;
static Transform s_camTransBackup;

void init(Game& game)
{
	game.entities = Entities(GAME_WORLD);
	game.entities.add_system<RenderSystem>(game.resources);
	game.entities.add_system<AnimationSystem>();
	game.entities.add_system<PhysicsSystem>();
	game.entities.add_system<AudioSystem>();
	game.entities.add_system<ModuleSystem>();
	ModuleSystem& modules = game.entities.get_system<ModuleSystem>();
#if EMBED_MODULES
	REGISTER_MODULE_FUNC(modules, asteroids);
	REGISTER_MODULE_FUNC(modules, devtools);
	REGISTER_MODULE_FUNC(modules, console);
	REGISTER_MODULE_FUNC(modules, fallingsand);
	REGISTER_MODULE_FUNC(modules, logo);
	REGISTER_MODULE_FUNC(modules, pong);
	REGISTER_MODULE_FUNC(modules, settings);
	REGISTER_MODULE_FUNC(modules, skyrunner);
	REGISTER_MODULE_FUNC(modules, testbed);
#endif
	modules.load(Engine::settings["modules"], false);
	game.entities.add_system<TriggerSystem>();
	game.entities.add_system<ImGuiSystem>(game.engine.window, game.engine.glContext);
	game.entities.get_system<ImGuiSystem>().applyDefaultStyle();
	game.scene = SceneLoader(game.entities);
	game.scene.load(game.scenePath, game.resources);
	Entity cameraEnt = game.entities.get_entity_by_tag("camera");
	ASSERT(cameraEnt.is_alive());
	Transform& camTrans = cameraEnt.has<Transform>() ? cameraEnt.get<Transform>() : cameraEnt.add<Transform>();
	Controller& controller = cameraEnt.add<Controller>(camTrans.position, camTrans.rotation);
	if (cameraEnt.has<RigidBody>()) {
		controller.body = cameraEnt.get<RigidBody>().body;
		if (!cameraEnt.has<GroundTracker>())
			cameraEnt.add<GroundTracker>();
	}
	if (game.restoreCam) {
		game.restoreCam = false;
		controller.position = s_controllerBackup.position;
		controller.rotation = s_controllerBackup.rotation;
		controller.angles = s_controllerBackup.angles;
		camTrans.position = s_camTransBackup.position;
		camTrans.rotation = s_camTransBackup.rotation;
		camTrans.dirty = true;
	}
	modules.call($id(INIT), &game);
}

int main(int argc, char* argv[])
{
	Args args(argc, argv);
	ECS::worlds = new Entities(GAME_WORLD);
	Game game { ECS::get(0) };
	Resources& resources = game.resources;
	resources.addPath(args.arg<string>(' ', "data", "../data/"));
	game.engine.init(resources.findPath(args.arg<string>('c', "config", "settings.json")));
	if (Engine::settings["moddir"].is_string())
		resources.addPath(Engine::settings["moddir"].string_value());
	game.engine.setIcon(resources.getImage("logo/weep-logo-32.png"));

	if (argc > 1 && argv[argc-1][0] != '-')
		game.scenePath = argv[argc-1];
	else if (Engine::settings["scene"].is_string())
		game.scenePath = Engine::settings["scene"].string_value();
	init(game);

	GifMovie gif("movie.gif", game.engine.width(), game.engine.height(), 10, false);

	bool running = true;
	bool gameControlsActive = false;
	bool screenshot = false;
	bool devtools = args.opt('d', "dev") || Engine::settings["devtools"].bool_value();
	SDL_Event e;
	while (running) {
		BEGIN_CPU_SAMPLE(MainLoop)
		BEGIN_GPU_SAMPLE(GPUFrame)
		RenderSystem& renderer = game.entities.get_system<RenderSystem>();
		PhysicsSystem& physics = game.entities.get_system<PhysicsSystem>();
		AudioSystem& audio = game.entities.get_system<AudioSystem>();
		AnimationSystem& animation = game.entities.get_system<AnimationSystem>();
		TriggerSystem& triggers = game.entities.get_system<TriggerSystem>();
		ModuleSystem& modules = game.entities.get_system<ModuleSystem>();
		ImGuiSystem& imgui = game.entities.get_system<ImGuiSystem>();
		Entity cameraEnt = game.entities.get_entity_by_tag("camera");
		Controller& controller = cameraEnt.get<Controller>();
		Camera& camera = cameraEnt.get<Camera>();
		Transform& cameraTrans = cameraEnt.get<Transform>();

		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				running = false;
				break;
			}
			if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE && e.window.windowID == SDL_GetWindowID(game.engine.window)) {
				running = false;
				break;
			}

			if (!gameControlsActive && imgui.processEvent(&e))
				continue;

			if (e.type == SDL_KEYUP) {
				SDL_Keysym keysym = e.key.keysym;

				if (keysym.sym == SDLK_ESCAPE) {
					if (gameControlsActive) {
						gameControlsActive = false;
						game.engine.grabMouse(false);
					} //else running = false;
				}

				if (keysym.sym == SDLK_RETURN && (keysym.mod & (KMOD_LALT | KMOD_RALT))) {
					game.engine.fullscreen(!game.engine.fullscreen());
					renderer.device().resizeRenderTargets(RenderDevice::RENDER_TARGET_SCREEN);
					continue;
				}
				else if (keysym.sym == SDLK_r && (keysym.mod & (KMOD_LCTRL|KMOD_LSHIFT)) == (KMOD_LCTRL|KMOD_LSHIFT)) {
					modules.call($id(devtools), $id(RELOAD_SHADERS), &game);
					continue;
				}
				else if (keysym.sym == SDLK_r && (keysym.mod & KMOD_LCTRL)) {
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
				else if (keysym.sym == SDLK_F9) {
					if (gif.recording) gif.finish();
					else {
						gif.frame.path = "movie_" + std::to_string(Engine::timems()) + ".gif";
						gif.startRecording();
					}
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
				gameControlsActive = !gameControlsActive;
				game.engine.grabMouse(gameControlsActive);
			}
			else if (e.type == SDL_MOUSEMOTION && gameControlsActive) {
				controller.angles.x += -0.05f * e.motion.yrel;
				controller.angles.y += -0.05f * e.motion.xrel;
			}

			modules.call($id(INPUT), &e);
		}

		imgui.newFrame(!gameControlsActive); // Needs to be after input events, otherwise at least mouse wheel does not work

		if (!imgui.usingKeyboard())
			controller.update(game.engine.dt);

		// Modules
		BEGIN_CPU_SAMPLE(moduleTime)
		modules.call($id(UPDATE), &game);
		END_CPU_SAMPLE()

		// Triggers
		BEGIN_CPU_SAMPLE(triggerTime)
		triggers.update(game.entities, game.engine.dt);
		END_CPU_SAMPLE()

		// Animation
		BEGIN_CPU_SAMPLE(animTime)
		animation.update(game.entities, game.engine.dt);
		END_CPU_SAMPLE()

		// Physics
		BEGIN_CPU_SAMPLE(physTime)
		physics.step(game.entities, game.engine.dt, game.engine.timeMult >= 1.f); // Use variable time step with time dilation...
		END_CPU_SAMPLE()

		if (cameraEnt.has<RigidBody>()) {
			//btRigidBody& body = *cameraEnt.get<RigidBody>().body;
			//camera.position = convert(body.getCenterOfMassPosition());
			if (cameraEnt.has<GroundTracker>())
				controller.onGround = cameraEnt.get<GroundTracker>().onGround;
		} else if (controller.enabled) {
			cameraTrans.position = controller.position;
		}
		if (controller.enabled)
			cameraTrans.rotation = controller.rotation;

		// Audio
		BEGIN_CPU_SAMPLE(audioTime)
		audio.update(game.entities, cameraTrans);
		END_CPU_SAMPLE()

		// Graphics
		BEGIN_GPU_SAMPLE(GPURender)
		BEGIN_CPU_SAMPLE(renderTimeMs)
		BEGIN_GPU_SAMPLE(Renderer)
		renderer.render(game.entities, camera, cameraTrans);
		END_GPU_SAMPLE()
		END_CPU_SAMPLE()

		if (devtools)
			modules.call($id(devtools), $id(DRAW_DEVTOOLS), &game);
		//ImGui::ShowTestWindow();
		//ImGui::ShowStyleEditor();

		// ImGui (profiler samples inside)
		imgui.render();

		if (screenshot) {
			START_MEASURE(screenshotMs)
			Image shot(Engine::width(), Engine::height(), 3);
			shot.screenshot();
			string path = "screenshot_" + std::to_string(Engine::timems()) + ".png";
			bool ret = shot.save(path.c_str());
			END_MEASURE(screenshotMs)
			if (ret) logInfo("Screenshot saved to %s (%.1fms)", path.c_str(), screenshotMs);
			else logError("Screenshot failed!");
			screenshot = false;
		}

		if (gif.recording)
			gif.recordFrame(game.engine.dt);

		END_GPU_SAMPLE() //GPURender

		BEGIN_CPU_SAMPLE(swap)
		game.engine.swap();
		END_CPU_SAMPLE()

		game.entities.update();

		modules.call($id(devtools), $id(FRAME_END), &game);

		if (game.reload) {
			if (game.restoreCam) {
				s_controllerBackup = controller;
				s_camTransBackup = cameraTrans;
			}
			modules.call($id(DEINIT), &game);
			renderer.reset(game.entities);
			physics.reset();
			game.scene.reset();
			resources.reset();
			init(game);
			game.reload = false;
		}
		END_GPU_SAMPLE()
		END_CPU_SAMPLE()
	}

	game.entities.get_system<RenderSystem>().reset(game.entities); // TODO: Should not be needed...

	game.entities.remove_system<ImGuiSystem>();
	game.entities.remove_system<TriggerSystem>();
	game.entities.remove_system<ModuleSystem>();
	game.entities.remove_system<AudioSystem>();
	game.entities.remove_system<PhysicsSystem>();
	game.entities.remove_system<AnimationSystem>();
	game.entities.remove_system<RenderSystem>();

	delete ECS::worlds; // TODO
	ModuleSystem::cleanUpHotloadFiles();

	game.engine.deinit();

	return EXIT_SUCCESS;
}
