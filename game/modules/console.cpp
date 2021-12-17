// Miscellaneous test stuff and a place to write temp code.

#include "common.hpp"
#include "gui.hpp"
#include "../game.hpp"
#include "imgui/imgui_stdlib.h"
#include <sstream>
#include <SDL_events.h>

static bool s_consoleOpen = false;
static std::string s_buffer;
static std::vector<std::string> s_history;
static int s_historyIndex = -1;

EXPORT void MODULE_FUNC_NAME(uint msg, void* param)
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
		case $id(INPUT):
		{
			SDL_Event& e = *static_cast<SDL_Event*>(param);
			if (e.type == SDL_KEYUP)
			{
				SDL_Keysym keysym = e.key.keysym;
				if (keysym.scancode == SDL_SCANCODE_GRAVE) {
					s_consoleOpen = !s_consoleOpen;
				}
			}
			break;
		}
		case $id(UPDATE):
		{
			if (!s_consoleOpen)
				break;

			// TODO: Implement history with ImGuiInputTextFlags_CallbackHistory
			if (!s_history.empty() && ImGui::IsKeyReleased(ImGuiKey_UpArrow)) {
				s_historyIndex = (s_historyIndex + 1) % s_history.size();
				s_buffer = s_historyIndex;
			}

			ImVec2 windowPos = ImGui::GetMainViewport()->Pos;
			ImVec2 windowSize = ImGui::GetMainViewport()->Size;
			ImGui::SetNextWindowPos(ImVec2(windowPos.x + 10, windowPos.y + windowSize.y - 16 - 10)); // TODO: Font height
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
			ImGui::Begin("##consolewindow", NULL, ImGuiSystem::MinimalWindow);
			ImGui::SetKeyboardFocusHere();
			if (ImGui::InputText("##console", &s_buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
				if (!s_buffer.empty()) {
					std::istringstream ss(s_buffer);
					std::string cmd, param;
					ss >> cmd >> param;

					if (bool* cvar = getCVar(cmd)) {
						bool newValue = false;
						bool ok = false;
						if (param == "0" || param == "false") { newValue = false; ok = true; }
						else if (param == "1" || param == "true") { newValue = true; ok = true; }
						if (ok) {
							*cvar = newValue;
							logInfo("Set cvar %s to %d", cmd.c_str(), *cvar);
						} else {
							logError("Could not parse parameter %s for cvar %s", param.c_str(), cmd.c_str());
						}
					} else {
						logError("Unknown cvar: %s", cmd.c_str());
					}

					s_history.push_back(s_buffer);
					s_historyIndex = -1;
					s_buffer = "";
				}
				s_consoleOpen = false;
			}
			ImGui::End();
			ImGui::PopStyleVar(2);
			break;
		}
	}
}
