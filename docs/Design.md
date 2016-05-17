Design Principles
=================

Game specific code should be in the modules, so the main `weep` executable is more like a generic player which can run multiple different games.

Use modern OpenGL with no regard to e.g. ES 2.0 compatibility. While no other rendering backends are planned (though might be fun to experiment with Vulkan), keep OpenGL code separated in the render device class and pull render API independent stuff to a higher level.

Avoid creating engine specific formats and prefer to load easily editable, widely used files such as obj, png and jpg directly. This also avoids the need for an asset build step. However, don't try to support everything under the sun, but instead focus on few good ones that get the job done.

Try to be cross-platform but actively only on Linux and Windows.

Avoid dependencies and prefer small, embeddable ones. Use no GPL licensed code (expect possibly with separate tools).

Use modern C++11 with STL, but no Boost. Also try to keep the amount of templates and inheritance at a minimum, and prefer simple solution instead of lots of abstraction levels etc.

