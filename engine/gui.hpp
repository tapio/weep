#pragma once
#include "common.hpp"
#include "imgui/imgui.h"

class ImGuiSystem : public System
{
public:
	ImGuiSystem();
	~ImGuiSystem();

	void newFrame();
	void processEvent(union SDL_Event* event);
	void applyDefaultStyle();
	void applyInternalState();

	ImFont* loadFont(const string& name, const string& path, float size);
	ImFont* getFont(const string& name) const;

private:
	void* m_imguiState = nullptr;
	std::map<string, ImFont*> m_fonts;
};
