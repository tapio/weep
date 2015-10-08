#pragma once
#include "common.hpp"
#include "imgui/imgui.h"

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
	ImFont* getFont(const string& name) const;

private:
	void* m_imguiState = nullptr;
	std::map<string, ImFont*> m_fonts;
};
