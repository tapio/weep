#pragma once
#include "common.hpp"
#include "imgui/imgui.h"
#include <unordered_map>

namespace ImGui {
	inline void SetNextWindowPosCenter() { SetNextWindowPos({0, 0}, 0, {0.5f, 0.5f}); }
};

class ImGuiSystem : public System
{
public:
	ImGuiSystem(struct SDL_Window* window, void* gl_context);
	~ImGuiSystem();

	NONCOPYABLE(ImGuiSystem);

	void newFrame(SDL_Window* window);
	bool processEvent(union SDL_Event* event);
	void render();
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
	ImGuiContext* m_imguiContext = nullptr;
	std::unordered_map<uint, ImFont*> m_fonts;
};


struct ScopedFont
{
	ScopedFont(Entities& entities, uint id) {
		ImFont* font = entities.get_system<ImGuiSystem>().getFont(id);
		ASSERT(font);
		ImGui::PushFont(font);
	}
	~ScopedFont() {
		ImGui::PopFont();
	}
};
