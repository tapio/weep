// Compute shader testing

#include "common.hpp"
#include "components.hpp"
#include "material.hpp"
#include "engine.hpp"
#include "renderer.hpp"
#include "glrenderer/renderdevice.hpp"
#include "../game.hpp"

static SSBO<vec3> posBuffer(0);

EXPORT void MODULE_FUNC_NAME(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			game.engine.moduleInit();

			posBuffer.buffer.resize(1024);
			posBuffer.create();
			posBuffer.upload();

			RenderSystem& renderer = game.entities.get_system<RenderSystem>();
			const ShaderProgram& compShader = renderer.device().getProgram($id(particletest));
			compShader.use();
			compShader.compute(posBuffer.buffer.size());

			break;
		}
		case $id(UPDATE):
		{
			break;
		}
	}
}
