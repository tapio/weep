#include "gui.hpp"
#include "engine.hpp"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include <SDL_events.h>


ImGuiSystem::ImGuiSystem(SDL_Window* window, void* gl_context)
{
	IMGUI_CHECKVERSION();
	m_imguiContext = ImGui::CreateContext();

	int attr = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &attr);
	bool gles = attr == SDL_GL_CONTEXT_PROFILE_ES;
	const char* glsl_version = gles
		? "#version 300 es\n"
		: "#version 330\n";

	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

ImGuiSystem::~ImGuiSystem()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiSystem::newFrame(SDL_Window* window)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();
}

bool ImGuiSystem::processEvent(SDL_Event* event)
{
	ImGui_ImplSDL2_ProcessEvent(event);
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

void ImGuiSystem::render()
{
	BEGIN_CPU_SAMPLE(imguiRenderTimeMs)
	ImGui::Render();
	BEGIN_GPU_SAMPLE(ImGuiRender)
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	END_GPU_SAMPLE()
	END_CPU_SAMPLE()
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
	ImGui::SetCurrentContext(m_imguiContext);
}

ImFont* ImGuiSystem::loadFont(const string& name, const string& path, float size)
{
	uint id = id::hash(name);
	if (m_fonts.find(id) != m_fonts.end()) {
		logWarning("Font %s already loaded", name.c_str());
		return m_fonts[id];
	}
	START_MEASURE(fontLoadTimeMs)
	ImGuiIO& io = ImGui::GetIO();
	ImFont* font = io.Fonts->AddFontFromFileTTF(path.c_str(), size, NULL, io.Fonts->GetGlyphRangesDefault());
	END_MEASURE(fontLoadTimeMs)
	if (!font) {
		logError("Failed to load font %s from \"%s\"", name.c_str(), path.c_str());
		return nullptr;
	}
	m_fonts[id] = font;
	logDebug("Loaded font %s from \"%s\" (size: %.1f) in %.1fms", name.c_str(), path.c_str(), size, fontLoadTimeMs);
	return font;
}

ImFont* ImGuiSystem::getFont(uint id) const
{
	auto it = m_fonts.find(id);
	if (it == m_fonts.end()) {
		logWarning("Can't find font id %d", id);
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
	//style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.50f, 0.78f, 0.90f, 0.50f);
	//style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.70f, 0.78f, 0.90f, 0.60f);
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
}
