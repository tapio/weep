// Falling sand experiment

#include "common.hpp"
#include "gui.hpp"
#include "image.hpp"
#include "glrenderer/texture.hpp"
#include "../game.hpp"

#include <glm/gtc/random.hpp>
#include <utility>

static Image worldImg(512, 512, 1);
static Texture worldTex;

inline unsigned char sample(const Image& img, int x, int y, unsigned char def = 1) {
	if (x < 0 || x >= img.width || y < 0 || y >= img.height)
		return def;
	return img.data[x + y * img.width];
}

inline bool tryMove(int x, int y, int dx, int dy) {
	const int newx = x + dx;
	const int newy = y + dy;
	if (sample(worldImg, newx, newy))
		return false;
	std::swap(worldImg.data[x + y * worldImg.width], worldImg.data[newx + newy * worldImg.width]);
	return true;
}


MODULE_EXPORT void MODULE_FUNC_NAME(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		case $id(RELOAD):
		{
			game.moduleInit();
			worldTex.minFilter = GL_NEAREST;
			worldTex.magFilter = GL_NEAREST;
			worldTex.destroy();
			worldTex.create();

			for (int y = 0; y < worldImg.height; ++y) {
				for (int x = 0; x < worldImg.width; ++x) {
					unsigned char pix = glm::linearRand(0.f, 1.f) > 0.75f ? 255 : 0;
					worldImg.data[x + y * worldImg.width] = pix;
				}
			}
			break;
		}
		case $id(UPDATE):
		{
			// Update falling sand simulation
			for (int y = worldImg.height - 1; y >= 0; --y) {
				for (int x = 0; x < worldImg.width; ++x) {
					if (tryMove(x, y, 0, 1))
						continue;
					if (tryMove(x, y, -1, 1))
						continue;
					if (tryMove(x, y, 1, 1))
						continue;
				}
			}

			// Update GPU texture
			worldTex.upload(worldImg);

			// Draw the thing
			ImVec2 windowPos = ImGui::GetMainViewport()->Pos;
			ImVec2 windowSize = ImGui::GetMainViewport()->Size;
			const float pad = 2;
			ImGui::SetNextWindowPos(ImVec2(windowPos.x + windowSize.x * 0.5f - worldImg.width * 0.5f, windowPos.y + windowSize.y * 0.5f - worldImg.height * 0.5f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(pad, pad));
			ImGui::Begin("##FallingSandWindow", NULL, ImGuiSystem::MinimalWindow);
			ImGui::Image(reinterpret_cast<ImTextureID>(worldTex.id), ImVec2(worldImg.width, worldImg.height));
			ImGui::End();
			ImGui::PopStyleVar();
			break;
		}
	}
}
