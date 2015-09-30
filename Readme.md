WeepEngine
==========

This is a small personal game engine. It's mainly a platform for me to experiment with OpenGL and perhaps build small game prototypes. While you can use it too under the MIT license, you probably shouldn't as it's incomplete, inefficient and buggy.

## Features

* OpenGL 4 forward renderer
	- Multiple point lights
	- One directional sun light
	- HDR with multiple tonemap functions
	- Bloom / glow
	- Diffuse / normal / specular / emission map support
	- Automatic mesh smoothing with tessellation shaders
	- #define based uber shader
* Mesh loading from Wavefront OBJ and heightmap images
* Entity-component based architecture
* Physics through Bullet dynamics library
* Hotloadable gameplay code modules
* JSON based configuration and scene declaration
* ImGui user interface integration
* Sound system

## Dependencies

You need C++11 capable compiler and CMake. A number of third-party libraries are included in the repository and built as a part of the build process (see "third-party" subfolder). In addition, you need SDL 2.0.2+ and OpenGL installed.

The core repository only includes some debug assets, so you probably also want to have "weep-media" repository.

## Building

	mkdir build
	cd build
	cmake ..
	make

## License

MIT, see LICENSE file for details.

