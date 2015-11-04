#include "common.hpp"
#include "gui.hpp"
#include "glrenderer/texture.hpp"
#include "../game.hpp"

static Texture logoTex;

EXPORT void ModuleFunc(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			game.engine.moduleInit();
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
			ImGui::SetNextWindowPos(ImVec2(10, ImGui::GetIO().DisplaySize.y - 32 - 10));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::Begin("##LogoWindow", NULL, ImVec2(0, 0), 0.f, ImGuiSystem::MinimalWindow);
			ImFont* font = game.entities.get_system<ImGuiSystem>().getFont($id(logo-font));
			ASSERT(font);
			ImGui::PushFont(font);
			ImGui::Image(reinterpret_cast<ImTextureID>(logoTex.id), ImVec2(32, 32));
			ImGui::SameLine();
			ImGui::Text("Powered by\nWeepEngine");
			ImGui::PopFont();
			ImGui::End();
			ImGui::PopStyleVar();
			break;
		}
	}
}
