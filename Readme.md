WeepEngine ![Logo](data/logo/weep-logo-32.png)
==============================================

This is a small personal game engine. It's mainly a platform for me to experiment with OpenGL and perhaps build small game prototypes. While you can use it too under the MIT license, you probably shouldn't as it's incomplete, inefficient, buggy and largely undocumented.

![Skyrunner](docs/pics/skyrunner.gif)
![Pong](docs/pics/pong.gif)  
![Testscene](docs/pics/testscene.gif)
![Debug](docs/pics/debug.gif)  

<!--
![Reflections](docs/pics/shot_1.jpg)
![Materials](docs/pics/shot_2.jpg)  
![Pong](docs/pics/shot_3.jpg)
![Level](docs/pics/shot_4.jpg)  
![Debug](docs/pics/shot_5.jpg)
![Skyrunner](docs/pics/shot_6.jpg)
-->

## Features

Many might be unpolished...

* OpenGL 4 forward renderer
	- One directional sun light with shadow mapping
	- Multiple point lights with omnidirectional shadow maps
	- HDR with multiple tonemap functions
	- Bloom / glow
	- Diffuse / normal / specular / emission / height / AO map support
	- Dynamic reflections with reflectivity map support
	- Automatic mesh smoothing with tessellation shaders
	- LODs
	- Postprocessing effects: vignette, sepia, saturation control, chromatic aberration...
	- #define based uber shader
	- Automatic shader permutation generation based on material properties
	- Automatic shader reload on file change
* Mesh loading from Wavefront .obj, Inter-Quake Model .iqm and heightmap images
* Basic skeletal animation (GPU skinning)
* Entity-component based architecture
* Physics through Bullet dynamics library
* Hotloadable gameplay code modules
* JSON based configuration and scene declaration
* ImGui user interface integration
* Sound system: positional audio, sample randomization, contact sounds
* Built-in screenshot and gif movie capture
* Runs on Linux and Windows (mostly)

## Known Issues

Here's some things that need work. The list is by no means exhaustive.

* There is little multi-threading going on
* Sun shadow map really needs cascades
* Light system is poor, should implement Forward+
* Tonemapping needs adaptive exposure
* Animation and sound systems are very basic
* Gameplay modules barely work on Windows and hotloading fails in some situations
* Entity/component destroying works poorly as it's currently implemented
* Windows support is fragile as I mainly develop this on a Linux box

## Dependencies

You need CMake and C++11 capable compiler. Currently Clang works best as GCC breaks live reload of non-trivial code plugins. Visual Studio is untested.

A number of third-party libraries are included in the repository and built as a part of the build process (see "third-party" subfolder). In addition, you need SDL 2.0.2+ and OpenGL drivers installed.

The core repository only includes some debug assets, so you probably also want to have `weep-media` repository (cloned as a sub folder next to this readme).

## Building

	mkdir build
	cd build
	cmake ..
	cmake --build .

## Running

Run "weep" from the build directory. You can give the scene to load as a command line argument or select it from the dev tools. See the Readme in `weep-media` repository for some example scenes.

## Acknowledgements

Renderer code, especially shader code owes a lot to various tutorials, among others:

* http://learnopengl.com/
* http://www.opengl-tutorial.org/
* http://www.sunandblackcat.com/other.php

## License

MIT, see LICENSE file for details.

