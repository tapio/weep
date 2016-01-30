// Implements a settings menu for end-user. This is in a separate module
// so that it can be shared between multiple small example games.

#include "common.hpp"
#include "renderer.hpp"
#include "glrenderer/renderdevice.hpp"
#include "audio.hpp"
#include "gui.hpp"
#include "../game.hpp"


EXPORT void ModuleFunc(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			game.engine.moduleInit();
			ImGuiSystem& imgui = game.entities.get_system<ImGuiSystem>();
			imgui.applyInternalState();
			break;
		}
		case $id(DRAW_SETTINGS_MENU):
		{
			RenderSystem& renderer = game.entities.get_system<RenderSystem>();
			AudioSystem& audio = game.entities.get_system<AudioSystem>();

			bool vsync = game.engine.vsync();
			bool oldVsync = vsync;
			ImGui::Checkbox("V-sync", &vsync);
			if (vsync != oldVsync)
				game.engine.vsync(vsync);

			ImGui::SameLine();
			bool fullscreen = game.engine.fullscreen();
			bool oldFullscreen = fullscreen;
			ImGui::Checkbox("Fullscreen", &fullscreen);
			if (fullscreen != oldFullscreen) {
				game.engine.fullscreen(fullscreen);
				renderer.device().resizeRenderTargets();
			}

			float volume = audio.soloud->getGlobalVolume();
			float oldVolume = volume;
			ImGui::SliderFloat("Volume", &volume, 0.f, 1.25f);
			if (volume != oldVolume)
				audio.soloud->setGlobalVolume(volume);

			ImGui::SliderInt("Threads", (int*)&game.engine.threads, 0, 8);

			int oldMsaa = Engine::settings["renderer"]["msaa"].number_value();
			float msaa = log2(oldMsaa);
			ImGui::SliderFloat("MSAA", &msaa, 0.f, 3.f, "%.0f");
			int newMsaa = powf(2, msaa);
			ImGui::SameLine(); ImGui::Text("%dx", newMsaa);
			if (newMsaa != oldMsaa) {
				// Aargh, I want mutable Json
				Json::object settings = Engine::settings.object_items();
				Json::object rendererSettings = settings["renderer"].object_items();
				rendererSettings["msaa"] = Json(newMsaa);
				settings["renderer"] = Json(rendererSettings);
				Engine::settings = Json(settings);
				renderer.device().resizeRenderTargets();
			}

			ImGui::Checkbox("FXAA", (bool*)&renderer.env().postAA);
			ImGui::SameLine();
			ImGui::Checkbox("Shadows", &renderer.settings.shadows);
			break;
		}
	}
}
