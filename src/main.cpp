#include "platform.hpp"
#include "geometry.hpp"
#include "renderer.hpp"

int main(int argc, char* argv[])
{
	Platform::init();

	Geometry geom = Geometry::createPlane(100, 100);
	GetRenderer().addGeometry(&geom);

	Platform::run();

	return 0;
}
