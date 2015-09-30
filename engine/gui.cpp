#include "gui.hpp"
#include "engine.hpp"
#include "imgui/imgui_impl_sdl_gl3.h"


ImGuiSystem::ImGuiSystem()
{
	ImGui_ImplSDLGL3_Init(Engine::window);
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

void ImGuiSystem::processEvent(SDL_Event* event)
{
	ImGui_ImplSDLGL3_ProcessEvent(event);
}

void ImGuiSystem::applyInternalState()
{
	ImGui::SetInternalState(m_imguiState);
}

ImFont* ImGuiSystem::loadFont(const string& name, const string& path, float size)
{
	if (m_fonts.find(name) != m_fonts.end()) {
		logWarning("Font %s already loaded as %s", name.c_str(), path.c_str());
		return m_fonts[name];
	}
	ImGuiIO& io = ImGui::GetIO();
	ImFont* font = io.Fonts->AddFontFromFileTTF(path.c_str(), size, NULL, io.Fonts->GetGlyphRangesDefault());
	m_fonts[name] = font;
	return font;
}

ImFont* ImGuiSystem::getFont(const string& name) const
{
	auto it = m_fonts.find(name);
	if (it == m_fonts.end()) {
		logWarning("Can't find font %s", name.c_str());
		return nullptr;
	}
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
