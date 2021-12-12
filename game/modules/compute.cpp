// Compute shader testing

#include "common.hpp"
#include "components.hpp"
#include "material.hpp"
#include "engine.hpp"
#include "geometry.hpp"
#include "renderer.hpp"
#include "glrenderer/renderdevice.hpp"
#include "../game.hpp"

EXPORT void MODULE_FUNC_NAME(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			game.engine.moduleInit();
			break;
		break;
		}
		case $id(UPDATE):
		{
			break;
		}
	}
}
