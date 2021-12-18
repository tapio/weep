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

static int textEditCallback(ImGuiInputTextCallbackData* data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) {
		if (data->EventKey == ImGuiKey_UpArrow) {
			if (s_historyIndex >= 0)
				--s_historyIndex;
			data->DeleteChars(0, data->BufTextLen);
			if (s_historyIndex >= 0 && s_historyIndex < s_history.size()) {
				data->InsertChars(0, s_history[s_historyIndex].c_str());
				data->SelectAll();
			}
		} else if (data->EventKey == ImGuiKey_DownArrow) {
			if (s_historyIndex < (int)s_history.size())
				++s_historyIndex;
			data->DeleteChars(0, data->BufTextLen);
			if (s_historyIndex >= 0 && s_historyIndex < s_history.size()) {
				data->InsertChars(0, s_history[s_historyIndex].c_str());
				data->SelectAll();
			}
		}
	} else if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion) {
		if (s_buffer.empty())
			return 0;
		auto candidates = CVarBase::getMatchingCVarNames(s_buffer);
		if (candidates.empty())
			return 0;
		// Single match - replace
		if (candidates.size() == 1) {
			if (candidates.back() != s_buffer) {
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, candidates.back().c_str());
			}
			return 0;
		}
		// Multiple matches - complete as much as we can
		int matchLen = (int)s_buffer.size();
		while (true) {
			int c = 0;
			bool allMatch = true;
			for (int i = 0; i < candidates.size() && allMatch; i++) {
				if (i == 0)
					c = toupper(candidates[i][matchLen]);
				else if (c == 0 || c != toupper(candidates[i][matchLen]))
					allMatch = false;
			}
			if (!allMatch)
				break;
			matchLen++;
		}
		if (matchLen > 0) {
			const char* selected = candidates.front().c_str();
			data->DeleteChars(0, data->BufTextLen);
			data->InsertChars(0, selected, selected + matchLen);
		}
	}
	return 0;
};

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

			ImVec2 windowPos = ImGui::GetMainViewport()->Pos;
			ImVec2 windowSize = ImGui::GetMainViewport()->Size;
			ImGui::SetNextWindowPos(ImVec2(windowPos.x + 10, windowPos.y + windowSize.y - 16 - 10)); // TODO: Font height
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
			ImGui::Begin("##consolewindow", NULL, ImGuiSystem::MinimalWindow);
			constexpr int textInputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackCompletion;
			ImGui::SetKeyboardFocusHere();
			if (ImGui::InputText("##console", &s_buffer, textInputFlags, textEditCallback)) {
				if (!s_buffer.empty()) {
					std::istringstream ss(s_buffer);
					std::string cmd, param;
					ss >> cmd >> param;

					if (CVarBase* cvar = CVarBase::getCVar(cmd)) {
						if (param.empty()) {
							logInfo("Value of cvar %s: %g", cmd.c_str(), cvar->value);
						} else if (cvar->tryParseFrom(param)) {
							logInfo("Set cvar %s to %g", cmd.c_str(), cvar->value);
							s_consoleOpen = false;
						} else {
							logError("Could not parse parameter %s for cvar %s", param.c_str(), cmd.c_str());
						}
					} else {
						logError("Unknown cvar: %s", cmd.c_str());
					}

					s_history.push_back(s_buffer);
					s_historyIndex = s_history.size();
					s_buffer = "";
				} else s_consoleOpen = false;
			}
			ImGui::End();
			ImGui::PopStyleVar(2);
			break;
		}
	}
}
