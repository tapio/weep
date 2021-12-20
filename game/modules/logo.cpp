// Renders Weep Engine logo to a screen corner.

#include "common.hpp"
#include "gui.hpp"
#include "glrenderer/texture.hpp"
#include "../game.hpp"

static Texture logoTex;

MODULE_EXPORT void MODULE_FUNC_NAME(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		case $id(RELOAD):
		{
			game.moduleInit();
			ImGuiSystem& imgui = game.entities.get_system<ImGuiSystem>();
			imgui.loadFont("logo-font", game.resources.findPath("fonts/Orbitron-Regular.ttf"), 14.f);
			break;
		}
		case $id(UPDATE):
		{
			if (!logoTex.valid()) {
				Image* logoImg = game.resources.getImage("logo/weep-logo-32.png");
				ASSERT(logoImg);
				logoTex.create();
				logoTex.upload(*logoImg);
			}
			ImVec2 windowPos = ImGui::GetMainViewport()->Pos;
			ImVec2 windowSize = ImGui::GetMainViewport()->Size;
			const float pad = 2;
			ImGui::SetNextWindowPos(ImVec2(windowPos.x + windowSize.x - 155, windowPos.y + windowSize.y - 32 - 10));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(pad, pad));
			ImGui::Begin("##LogoWindow", NULL, ImGuiSystem::MinimalWindow);
			ImFont* font = game.entities.get_system<ImGuiSystem>().getFont($id(logo-font));
			ASSERT(font);
			ImGui::PushFont(font);
			ImGui::Image(reinterpret_cast<ImTextureID>(logoTex.id), ImVec2(32, 32));
			ImGui::SameLine();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Powered by  \nWeepEngine");
			ImGui::PopFont();
			ImGui::End();
			ImGui::PopStyleVar();
			break;
		}
	}
}
