Code Modules
============

In Weep engine, it is encouraged to put game specific code to "module" files that are then loaded through scene files. Modules are essentially .cpp files compiled to dynamic libraries (.dll/.so/.dylib) that are loaded at run time and can generally also be reloaded on the fly, accelerating iteration.

A module must export the following function:

	EXPORT void ModuleFunc(uint msg, void* param);

However, to support embedded modules, it should be written as follows:

	EXPORT void MODULE_FUNC_NAME(uint msg, void* param);

The engine uses this function to communicate with the module. The msg param is a compile time hash (e.g. `$id(INIT)`) which identifies what the module is expected to do, and the void* param is interpreted depending on the message. Modules can also send custom messages to each other through the ModuleSystem's `call` function. Handling messages is optional and unknown messages should be ignored.

Messages that the engine sends automatically to all active modules:

* **INIT**, param: Game*
	* Sent when the module is loaded
	* You could e.g. store the Game pointer to a static variable for later use
* **RELOAD**, param: Game*
	* Sent when the module code is hotloaded (but not the whole game)
	* Generally should probably run most of the same stuff as **INIT** (e.g. static variables are reset on reload)
* **INPUT**, param: SDL_Event*
	* Sent for each SDL event unless the event was handled at the engine level
* **UPDATE**, param: Game*
	* Sent once per frame, after input messages
* **DEINIT**, param: Game*
	* Sent when the module is unloaded on game exit or full scene reload
	* Not sent on module-only reload (see **RELOAD**)


Embedded Modules
----------------

Due to various difficulties with DLL boundaries, the modules don't always work well. Also, using a debugger is somewhat easier with just one binary.
As such, Weep also supports building modules inside the executable, in which case they cannot be hotloaded, but the API and usage from code is identical.

To support this, main.cpp needs to know about and register the embedded modules, which is done by two lines of macros:

	DECLARE_MODULE_FUNC(mymodulename);

	REGISTER_MODULE_FUNC(modules, mymodulename);

See main.cpp for more details.
