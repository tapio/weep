// Falling sand experiment

#include "common.hpp"
#include "gui.hpp"
#include "image.hpp"
#include "glrenderer/texture.hpp"
#include "../game.hpp"

#include <glm/gtc/random.hpp>
#include <utility>

enum class MatType {
	Empty,
	Static,
	Powder,
	Liquid,
	Gas
};

struct MatDef {
	const char* name = "";
	MatType type = MatType::Powder;
	uint color = 0xffffffff;
};
static MatDef materials[] = {
	{
		"Empty",
		MatType::Empty,
		0x00000000
	},
	{
		"Ground",
		MatType::Static,
		0xff0088cc
	},
	{
		"Sand",
		MatType::Powder,
		0xff00ffff
	},
	{
		"Water",
		MatType::Liquid,
		0xffff0000
	},
	{
		"Steam",
		MatType::Gas,
		0xaaaaaaaa
	}
};
constexpr uint NumMaterials = countof(materials);

static Image worldBuf(512, 512, 1);
static Image worldImg(worldBuf.width, worldBuf.height, 4);
static Texture worldTex;

inline unsigned char sample(const Image& img, int x, int y, unsigned char def = 1) {
	if (x < 0 || x >= img.width || y < 0 || y >= img.height)
		return def;
	return img.data[x + y * img.width];
}

inline bool tryMove(MatType mat, int x, int y, int dx, int dy) {
	const int newx = x + dx;
	const int newy = y + dy;
	unsigned char targetMatId = sample(worldBuf, newx, newy, 255);
	if (targetMatId == 255) // Invalid bounds blocks
		return false;
	MatType targetMat = materials[targetMatId].type;
	if (targetMat == MatType::Static || targetMat == MatType::Powder) // Static and powder always blocks
		return false;
	if (targetMat == mat) // Same material blocks
		return false;
	std::swap(worldBuf.data[x + y * worldBuf.width], worldBuf.data[newx + newy * worldBuf.width]);
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

			// Initialize world
			for (int y = 0; y < worldBuf.height; ++y) {
				for (int x = 0; x < worldBuf.width; ++x) {
					unsigned char matId = glm::linearRand(0u, NumMaterials-1);
					worldBuf.data[x + y * worldBuf.width] = matId;
				}
			}
			break;
		}
		case $id(UPDATE):
		{
			// Update falling sand simulation
			// Bottom to top, alternate between left-right / right-left
			static bool leftToRight = true;
			int xStart = leftToRight ? 0 : (worldBuf.width - 1);
			int xEnd = leftToRight ? worldBuf.width : -1;
			int xStep = leftToRight ? 1 : -1;
			int xStep2 = leftToRight ? -1 : 1;
			leftToRight = !leftToRight;
			for (int y = worldBuf.height - 1; y >= 0; --y) {
				for (int x = xStart; x != xEnd; x += xStep) {
					MatDef mat = materials[sample(worldBuf, x, y, 0)];
					if (mat.type == MatType::Powder) {
						if (tryMove(mat.type, x, y, 0, 1))
							continue;
						if (tryMove(mat.type, x, y, xStep, 1))
							continue;
						if (tryMove(mat.type, x, y, xStep2, 1))
							continue;
					}
					else if (mat.type == MatType::Liquid) {
						if (tryMove(mat.type, x, y, 0, 1))
							continue;
						if (tryMove(mat.type, x, y, xStep, 0))
							continue;
						if (tryMove(mat.type, x, y, xStep2, 0))
							continue;
					}
					else if (mat.type == MatType::Gas) {
						if (tryMove(mat.type, x, y, 0, -1))
							continue;
						if (tryMove(mat.type, x, y, xStep, 0))
							continue;
						if (tryMove(mat.type, x, y, xStep2, 0))
							continue;
					}
				}
			}

			// Update image pixels
			uint rowBytes = worldImg.width * worldImg.channels;
			for (int y = 0; y < worldImg.height; ++y) {
				for (int x = 0; x < worldImg.width; ++x) {
					unsigned char matId = worldBuf.data[x + y * worldBuf.width];
					uint* pix = (uint*)&worldImg.data[(x * worldImg.channels) + y * rowBytes];
					*pix = materials[matId].color;
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
