#include "gui.hpp"
#include "engine.hpp"
#include "imgui/imgui_impl_sdl_gl3.h"
#include "SDL2/SDL_events.h"


ImGuiSystem::ImGuiSystem(SDL_Window* window)
{
	ImGui_ImplSDLGL3_Init(window);
	m_imguiState = ImGui::GetInternalState();
}

ImGuiSystem::~ImGuiSystem()
{
	ImGui_ImplSDLGL3_Shutdown();
}

void ImGuiSystem::newFrame()
{
	ImGui_ImplSDLGL3_NewFrame();
}

bool ImGuiSystem::processEvent(SDL_Event* event)
{
	ImGui_ImplSDLGL3_ProcessEvent(event);
	ImGuiIO& io = ImGui::GetIO();
	switch (event->type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		case SDL_TEXTEDITING:
		case SDL_TEXTINPUT:
			return io.WantCaptureKeyboard;
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEWHEEL:
			return io.WantCaptureMouse;
	}
	return false;
}

bool ImGuiSystem::usingMouse() const
{
	return ImGui::GetIO().WantCaptureMouse;
}

bool ImGuiSystem::usingKeyboard() const
{
	return ImGui::GetIO().WantCaptureKeyboard;
}

void ImGuiSystem::applyInternalState()
{
	ImGui::SetInternalState(m_imguiState);
}

ImFont* ImGuiSystem::loadFont(const string& name, const string& path, float size)
{
	if (m_fonts.find(name) != m_fonts.end()) {
		logWarning("Font %s already loaded", name.c_str());
		return m_fonts[name];
	}
	ImGuiIO& io = ImGui::GetIO();
	ImFont* font = io.Fonts->AddFontFromFileTTF(path.c_str(), size, NULL, io.Fonts->GetGlyphRangesDefault());
	if (!font) {
		logError("Failed to load font %s from \"%s\"", name.c_str(), path.c_str());
		return nullptr;
	}
	m_fonts[name] = font;
	logDebug("Loaded font %s from \"%s\" (size: %.1f)", name.c_str(), path.c_str(), size);
	return font;
}

ImFont* ImGuiSystem::getFont(const string& name) const
{
	auto it = m_fonts.find(name);
	if (it == m_fonts.end()) {
		logWarning("Can't find font %s", name.c_str());
		return nullptr;
	}
	ASSERT(it->second);
	return it->second;
}

void ImGuiSystem::applyDefaultStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 6.f;
	style.FrameRounding = 4.f;
	style.ScrollbarRounding = 5.f;
	style.GrabRounding = 3.f;
	style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.50f, 0.86f, 1.00f, 0.45f);
	style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.40f, 0.78f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.50f, 0.75f, 1.00f, 0.55f);
	style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.40f, 0.55f, 0.55f, 0.80f);
	style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.13f, 0.13f, 0.13f, 0.67f);
	style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.27f, 0.27f, 0.27f, 0.67f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.40f, 0.40f, 0.40f, 0.67f);
	style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.53f, 0.53f, 0.53f, 0.67f);
	style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.53f, 0.53f, 0.53f, 0.67f);
	style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(1.00f, 1.00f, 1.00f, 0.67f);
	style.Colors[ImGuiCol_Button]                = ImVec4(0.25f, 0.38f, 0.00f, 0.67f);
	style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.44f, 0.59f, 0.00f, 0.67f);
	style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.09f, 0.19f, 0.00f, 0.67f);
	style.Colors[ImGuiCol_Header]                = ImVec4(0.40f, 0.78f, 0.90f, 0.45f);
	style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.45f, 0.78f, 0.90f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.53f, 0.71f, 0.78f, 0.80f);
	style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.50f, 0.78f, 0.90f, 0.50f);
	style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.70f, 0.78f, 0.90f, 0.60f);
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
}
