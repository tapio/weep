#include "common.hpp"
#include "physics.hpp"


EXPORT void ModuleFunc(uint msg, void* param)
{
	switch (msg) {
		case $id(INIT):
		{
			//Entities& entities = *static_cast<Entities*>(param);
			// TODO: Generate level :)
			break;
		}
	}
}

