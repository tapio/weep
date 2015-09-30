#include "common.hpp"
#include "gui.hpp"
#include "glrenderer/texture.hpp"
#include "../game.hpp"

static const int MinimalWindow =
	ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|
	ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings;

static Texture logoTex;

EXPORT void ModuleFunc(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			ImGuiSystem& imgui = game.entities.get_system<ImGuiSystem>();
			imgui.applyInternalState();
			imgui.loadFont("logo-font", game.resources.findPath("fonts/Orbitron-Regular.ttf"), 14.f);
			break;
		}
		case $id(UPDATE):
		{
			if (!logoTex.valid()) {
				Image* logoImg = game.resources.getImage("logo/weep-logo-32.png");
				logoTex.create();
				logoTex.upload(*logoImg);
			}
			//ImGui::SetNextWindowPos(ImVec2(20, 20));
			//ImGui::Begin("", NULL, MinimalWindow);
			ImFont* font = game.entities.get_system<ImGuiSystem>().getFont("logo-font");
			ASSERT(font);
			ImGui::PushFont(font);
			ImGui::Image(reinterpret_cast<ImTextureID>(logoTex.id), ImVec2(32, 32));
			ImGui::SameLine();
			ImGui::Text("Powered by\nWeepEngine");
			ImGui::PopFont();
			//ImGui::End();
			break;
		}
	}
}
