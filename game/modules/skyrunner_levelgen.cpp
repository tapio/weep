#include "common.hpp"
#include "physics.hpp"


EXPORT void ModuleFunc(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			// TODO: Generate level :)
			break;
		}
	}
}

