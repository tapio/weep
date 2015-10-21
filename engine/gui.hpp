#pragma once
#include "common.hpp"
#include "imgui/imgui.h"
#include <unordered_map>

class ImGuiSystem : public System
{
public:
	ImGuiSystem(struct SDL_Window* window);
	~ImGuiSystem();

	NONCOPYABLE(ImGuiSystem);

	void newFrame();
	bool processEvent(union SDL_Event* event);
	bool usingMouse() const;
	bool usingKeyboard() const;
	void applyDefaultStyle();
	void applyInternalState();

	ImFont* loadFont(const string& name, const string& path, float size);
	ImFont* getFont(uint id) const;

	static const int MinimalWindow =
		ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|
		ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings;

private:
	void* m_imguiState = nullptr;
	std::unordered_map<uint, ImFont*> m_fonts;
};
