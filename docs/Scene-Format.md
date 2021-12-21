Weep Scene Format
=================

Scenes in Weep engine are described as a set of JSON files. This document attempts to document how things are organized and what properties are available. Currently scenes must mostly be authored by hand editing the JSON, although there is an incomplete Blender exporter in tools.

## Top level properties

Each json file should contain an object "{}" which can have one or more of the properties listed below. Order of the top level properties does not matter, although it makes sense to have them in the same order as here since that's the order they are processed.

* _"include"_: string or an array of strings referencing other json files that should be processed before interpreting the current file
* _"modules"_: array of strings containing the names of the code modules that should be loaded
* _"environment"_: environment configuration object (see below)
* _"fonts"_: font configuration object (see below)
* _"sounds"_: sound event configuration object (seel below)
* _"prefabs"_: object containing prefab definitions (objects that are not instantiated directly but rather act as prototypes for object instantiations); each key is the name of the prefab and the value follow regular object spec (see below)
* _"objects"_: array containing list if object instantiations (see below)

## Value formats

Whenever a property is listed to expect a vec2 or vec3, the value can be given either as an array of numbers or as a single number in which case all components use the given value. Colors can additionally be given as a hex string, either full form "#ff00ff" or shortened "#f0f". If a string is said to be hashed, it means that it's accessed from code with `$id(this-is-the-string)` syntax (resulting in uints instead of actual strings).

## Environment

* _"skybox"_: string (path to a skybox image or folder containing px.jpg, nx.jpg etc.) or array (paths to six side images) or null/undefined (procedural sky)
* _"exposure"_: float
* _"shadowDarkness"_: float
* _"bloomThreshold"_: float
* _"bloomIntensity"_: float
* _"tonemap"_: int
* _"ambient"_: color
* _"sunPosition"_: vec3
* _"sunColor"_: color
* _"fogColor"_: color
* _"fogDensity"_: float

## Fonts

Listed fonts can be used with the GUI system. Keys are hashed. Example:

	"fonts": {
		"title_font": {
			"path": "fonts/foo.ttf",
			"size": 48
		},
		"menu_font": {
			"path": "fonts/bar.ttf",
			"size": 24
		}
	}

## Sounds

The keys of the "sounds" object are ids (hashed) for sound events. The values for these keys are either strings (path to a .ogg sound file) or array of strings (multiple paths, one picked at random when the event id is played).

## Prefabs

These are read but no actual entities are generated. Instead they can be used as a template for actual objects. Format of individual prefab is the same as object defs specified below.

## Objects

Each object (entity) can have the following properties (most are optional):

* _"prefab"_: name of the prefab (key to a prefab object); if some property is not given in the object itself, a value from the prefab is used instead (if available)
* _"name"_: string, used for being able to retrieve the entity if needed, as well as for debugging purposes
* _"position"_: vec3
* _"rotation"_: vec3 (euler angles) or vec4 (xyzw quaternion)
* _"scale"_: vec3
* _"light"_: light emitter configuration object
	* _"type"_: string, only "point" supported currently
	* _"color"_: color
	* _"intensity"_: float, multiplier for color, default: 1.0
	* _"distance"_: float
	* _"shadowDistance"_: float, override shadow distance (defaults to light distance, use 0 to disable shadow)
	* _"decay"_: float, exponential light decay (1 = linear, 2 = quadratic...)
	* _"spotAngles"_: inner and outer spot light angle in degrees
* _"geometry"_: one of:
	1. string path to .png or .jpg image to create heightmap from
	2. string path to .obj or .iqm mesh
	3. array of objects for specifying LODs: keys are paths to meshes and values are numbers specifying the furthest distance the LOD object is visible from
* _material_: material configuration object
	* _"shaderName"_: string, name of the shader to use; leave out to use automatic über shader (recommended)
	* _"tessellate"_: bool, activate tessellation (default: false)
	* _"castShadow"_: bool, enable shadow casting (default: true)
	* _"receiveShadow"_: bool, enable shadow receiving (default: true)
	* _"drawReflection"_: bool, enable drawing in reflection probes (default: true)
	* _"animated"_: bool, needed for animated objects (default: false)
	* _"alphaTest"_: float, activate alpha testing with given threshold, i.e. larger or equal values pass (default: 0)
	* _"ambient"_: color
	* _"diffuse"_: color
	* _"specular"_: color
	* _"emissive"_: color
	* _"roughness"_: float, PBR surface roughness, 0 = perfectly smooth, 1 = totally rough
	* _"metalness"_: float (default: 0), 0 = dielectric, 1 = metallic PBR material
	* _"shininess"_: float (default: 0), specular exponent in classical phong shading
	* _"reflectivity"_: float (default: 0), needed to enable reflections
	* _"parallax"_: float (default: 0), needed to enable parallax mapping
	* _"uvOffset"_: vec2
	* _"uvRepeat"_: vec2
	* _"particleSize"_: vec2 (default: 0.01), size of quads when material is used with particles
	* _"diffuseMap"_: string
	* _"specularMap"_: string
	* _"emissionMap"_: string
	* _"normalMap"_: string
	* _"heightMap"_: string, needed to enable parallax mapping
	* _"aoMap"_: string
	* _"reflectionMap"_: string
* _"particles"_: particle emitter configuration object
	* _"count"_: int, maximum amount of particles
	* _"compute"_: string, name of the compute shader to use for simulating the particles
	* _"emit"_: bool, can be used for disabling new particle emission by setting to false (default: true)
	* _"localSpace"_: bool, if false, will simulate in world space, i.e. emitted particles will not follow emitter transform (default: false)
	* _"directionality"_: float, from 0 to 1, 0 means emit omni directionally, 1 means only emit to entity forward direction (default: 0)
	* _"randomRotation"_: float, from 0 to 1, how much random (view space billboard) rotation is applied to the particle quad
	* _"emitRadiusMinMax"_: vec2, min and max value for emission sphere radius
	* _"lifeTimeMinMax"_: = vec2, min and max life time for newly emitted particles 
	* _"speedMinMax"_: = vec2, min and max speed for newly emitted particles
* _"body"_: physics body configuration object
	* _"mass"_: float, use 0 or leave out for static objects
	* _"shape"_: string: "box", "sphere", "cylinder", "capsule", "trimesh"
	* _"geometry"_: string, if using "trimesh" shape this can be a path to a separate collision mesh (if missing, graphical trimesh is used instead)
	* _"friction"_: float
	* _"rollingFriction"_: float
	* _"restitution"_: float
	* _"noSleep"_: bool, disable sleeping
	* _"angularFactor"_: vec3
	* _"linearFactor"_: vec3
	* _"noGravity"_: bool, disable gravity
* _"animation"_: animation configuration object (animation itself must be in the geometry)
	* _"speed"_: float
	* _"play"_: bool, start playing immediately
* _"trackGround"_: bool, enable GroundTracker component
* _"trackContacts"_: bool, enable ContactTracker component
* _"triggerGroup"_: int, which trigger group 0-31 this entity belongs to (if any)
* _"triggerVolume"_: trigger volume configuration object
	* _"times"_: how many times this volume can be triggered (default is infinite)
	* _"radius"_: float, volume radius
	* _"groups"_: int or array of ints, which trigger groups can trigger this volume
	* _"receiver"_: string (hashed), name of the module to send the trigger messages
	* _"enterMessage"_: string (hashed), message to send when an entity enters the volume
	* _"exitMessage"_: string (hashed), message to send when an entity exits the volume
* _"moveSound"_: move sound configuration object
	* _"event"_: string (hashed), id of the sound event to play when moving
	* _"step"_: float, step length in meters (interval to play the sounds)
* _"contactSound"_: contact sound configuration object (requires ContactTracker)
	* _"event"_: string (hashed), id of the sound event to play when moving


