WeepEngine
==========

This is a small personal game engine. It's mainly a platform for me to experiment with OpenGL and perhaps build small game prototypes. While you can use it too under the MIT license, you probably shouldn't as it's incomplete, inefficient and buggy.

## Features

* OpenGL 4 forward renderer
	- One directional sun light with shadow mapping
	- Multiple point lights with omnidirectional shadow maps
	- HDR with multiple tonemap functions
	- Bloom / glow
	- Diffuse / normal / specular / emission / AO map support
	- Skybox reflections with reflectivity map support
	- Automatic mesh smoothing with tessellation shaders
	- #define based uber shader
	- Automatic shader permutation generation based on material properties
* Mesh loading from Wavefront .obj, Inter-Quake Model .iqm and heightmap images
* Skeletal animation (GPU skinning)
* Entity-component based architecture
* Physics through Bullet dynamics library
* Hotloadable gameplay code modules
* JSON based configuration and scene declaration
* ImGui user interface integration
* Sound system
* Runs on Linux and Windows (mostly)

## Dependencies

You need C++11 capable compiler and CMake. A number of third-party libraries are included in the repository and built as a part of the build process (see "third-party" subfolder). In addition, you need SDL 2.0.2+ and OpenGL installed.

The core repository only includes some debug assets, so you probably also want to have "weep-media" repository.

## Building

	mkdir build
	cd build
	cmake ..
	make

## Acknowledgements

Renderer code, especially shader code owes a lot to various tutorials, among others:

* http://learnopengl.com/
* http://www.opengl-tutorial.org/
* http://www.sunandblackcat.com/other.php?l=eng

## License

MIT, see LICENSE file for details.

