project(WeepEngine)
cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.1)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT WIN32)
	set(EMBED_MODULES_DEFAULT OFF)
else()
	set(EMBED_MODULES_DEFAULT ON)
endif()

set(EXE_NAME "weep" CACHE STRING "Executable name")
set(INSTALL_DIR "weep" CACHE STRING "Installation directory name")
option(SHIPPING_BUILD "Remove debug stuff" OFF)
option(USE_REMOTERY "Use Remotery profiler" ON)
option(USE_GLES "Link against OpenGL ES" OFF)
option(USE_LIBCXX "Use LLVM libc++ with Clang" OFF)
option(EMBED_MODULES "Embed plugin modules into the executable instead of using hotloadable DLLs" ${EMBED_MODULES_DEFAULT})

# Avoid source tree pollution
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
endif()

# Add a sensible build type default and warning because empty means no optimization and no debug info.
if(NOT CMAKE_BUILD_TYPE)
	message("WARNING: CMAKE_BUILD_TYPE is not defined!\n         Defaulting to CMAKE_BUILD_TYPE=RelWithDebInfo.\n         Use ccmake to set a proper value.")
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

if(EMBED_MODULES)
	message("Embedding plugin modules inside exe.")
else()
	message("Hot-loadable plugin modules enabled.")
endif()

if (MSVC)
	# Too noisy type conversion warnings
	add_compile_options(/wd4244)
	add_compile_options(/wd4267)
endif()

# Enable C++17 support and other compiler flags...
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	message(STATUS "${CMAKE_CXX_COMPILER_ID} detected, adding compile flags")
	set(COMMON_FLAGS "-std=c++17 -ffast-math -march=native -Wall -Wextra -Wno-unused-parameter")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}")
	if(WIN32)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--export-all-symbols")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	endif()
	if (USE_LIBCXX AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		message("Using LLVM libc++")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
	endif()
else()
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED YES)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
link_directories(${PROJECT_BINARY_DIR})

if(WIN32)
	add_definitions(-DNOMINMAX)
	include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/third-party/SDL2-win/include")
	link_directories("${CMAKE_SOURCE_DIR}/third-party/SDL2-win/lib/x64")
	#link_directories("${CMAKE_SOURCE_DIR}/third-party/SDL2-win/lib/x86")
	list(APPEND LIBS SDL2 SDL2main)
	file(COPY "${CMAKE_SOURCE_DIR}/third-party/SDL2-win/lib/x64/SDL2.dll" DESTINATION "${PROJECT_BINARY_DIR}")
else()
	find_package(PkgConfig)
	pkg_check_modules(SDL2 REQUIRED sdl2)
	include_directories(SYSTEM ${SDL2_INCLUDE_DIRS})
	link_directories(${SDL2_LIBRARY_DIRS})
	list(APPEND LIBS ${SDL2_LIBRARIES})
endif()

if(USE_GLES)
	find_library(GLES_LIBRARY NAMES GLESv3 GLESv2 mali)
	list(APPEND LIBS ${GLES_LIBRARY})
else()
	find_package(OpenGL REQUIRED)
	list(APPEND LIBS ${OPENGL_gl_LIBRARY})
endif()

if(USE_REMOTERY)
	add_definitions(-DUSE_PROFILER=1 -DRMT_USE_OPENGL=1 -DRMT_USE_POSIX_THREADNAMES=1)
	if(WIN32)
		list(APPEND LIBS "ws2_32" "winmm")
	else()
		list(APPEND LIBS "dl")
	endif()
endif()

include_directories(engine)
include_directories(SYSTEM third-party)
include_directories(SYSTEM third-party/bullet)
include_directories(SYSTEM third-party/soloud/include)

add_definitions(-DGL_GLEXT_PROTOTYPES)
# SoLoud
add_definitions(-DWITH_NULL -DWITH_SDL2_STATIC)

macro(handle_dep LIB)
	file(GLOB_RECURSE ${LIB}_SOURCES "third-party/${LIB}/*.cpp" "third-party/${LIB}/*.c")
	file(GLOB_RECURSE ${LIB}_HEADERS "third-party/${LIB}/*.hpp" "third-party/${LIB}/*.h")
	set_source_files_properties(${LIB}_HEADERS PROPERTIES HEADER_FILE_ONLY TRUE)
	if(${LIB}_SOURCES)
		add_library(${LIB} STATIC ${${LIB}_HEADERS} ${${LIB}_SOURCES})
	else()
		add_library(${LIB} INTERFACE)
		target_sources(${LIB} INTERFACE ${${LIB}_HEADERS})
	endif()
	list(APPEND DEPS ${LIB})
endmacro()

handle_dep(bullet)
handle_dep(ecs)
handle_dep(gih-h)
handle_dep(GL)
handle_dep(glad)
handle_dep(glm)
handle_dep(imgui)
handle_dep(iqm)
handle_dep(json11)
handle_dep(remotery)
handle_dep(soloud)
handle_dep(stb_image)

file(GLOB_RECURSE ENGINE_SOURCES "engine/*.cpp" "engine/*.hpp")
file(GLOB         GAME_SOURCES "game/*.cpp" "game/*.hpp")
file(GLOB         MODULES_SOURCES "game/modules/*.cpp")
file(GLOB_RECURSE SHADER_SOURCES "data/shaders/*.*")
file(GLOB_RECURSE CONFIG_SOURCES "data/*.json")
set_source_files_properties(${SHADER_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(${CONFIG_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)

if (USE_GLES)
	list(APPEND ENGINE_SOURCES "engine/glrenderer/gles_stub.c")
endif()

set(GAME_MAIN "game/main.cpp")
list(REMOVE_ITEM GAME_SOURCES "${PROJECT_SOURCE_DIR}/${GAME_MAIN}")

macro(set_game_defs TARGET)
	if(SHIPPING_BUILD)
		target_compile_definitions(${TARGET} PRIVATE -DSHIPPING_BUILD=1)
	endif()
	if(EMBED_MODULES)
		target_compile_definitions(${TARGET} PRIVATE -DEMBED_MODULES=1)
	endif()
endmacro()

add_library(engine STATIC ${ENGINE_SOURCES})
add_library(game STATIC ${GAME_SOURCES})
set_game_defs(engine)
set_game_defs(game)

macro(set_props TARGET)
	set_target_properties(${TARGET} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY                "${PROJECT_BINARY_DIR}"
		RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${PROJECT_BINARY_DIR}"
		RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_BINARY_DIR}"
		RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${PROJECT_BINARY_DIR}"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${PROJECT_BINARY_DIR}"
		LIBRARY_OUTPUT_DIRECTORY                "${PROJECT_BINARY_DIR}"
		LIBRARY_OUTPUT_DIRECTORY_DEBUG          "${PROJECT_BINARY_DIR}"
		LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_BINARY_DIR}"
		LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL     "${PROJECT_BINARY_DIR}"
		LIBRARY_OUTPUT_DIRECTORY_RELEASE        "${PROJECT_BINARY_DIR}"
	)
endmacro()

add_executable(${EXE_NAME} ${GAME_MAIN} ${SHADER_SOURCES} ${CONFIG_SOURCES})
set_props(${EXE_NAME})
set_game_defs(${EXE_NAME})
set_target_properties(${EXE_NAME} PROPERTIES ENABLE_EXPORTS ON)
set_target_properties(${EXE_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON) # CMake 3.4
set_target_properties(${EXE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
target_link_libraries(${EXE_NAME} PUBLIC game engine ${DEPS} ${LIBS})
install(TARGETS ${EXE_NAME} DESTINATION ${INSTALL_DIR}/bin)

# Game module plugins
if (EMBED_MODULES)
	foreach(GAME_MODULE_FILE ${MODULES_SOURCES})
		get_filename_component(GAME_MODULE_NAME ${GAME_MODULE_FILE} NAME_WE)
		add_library(${GAME_MODULE_NAME} STATIC ${GAME_MODULE_FILE})
		target_compile_definitions(${GAME_MODULE_NAME} PRIVATE -DMODULE_FUNC_NAME=ModuleFunc_${GAME_MODULE_NAME})
		set_game_defs(${GAME_MODULE_NAME})
		list(APPEND EMBEDDED_LIBS ${GAME_MODULE_NAME})
	endforeach()
	target_link_libraries(${EXE_NAME} PRIVATE ${EMBEDDED_LIBS} game engine ${DEPS} ${LIBS})
else()
	foreach(GAME_MODULE_FILE ${MODULES_SOURCES})
		get_filename_component(GAME_MODULE_NAME ${GAME_MODULE_FILE} NAME_WE)
		add_library(${GAME_MODULE_NAME} MODULE ${GAME_MODULE_FILE})
		target_compile_definitions(${GAME_MODULE_NAME} PRIVATE -DMODULE_NAME=${GAME_MODULE_NAME})
		set_props(${GAME_MODULE_NAME})
		set_game_defs(${GAME_MODULE_NAME})
		target_link_libraries(${GAME_MODULE_NAME} ${EXE_NAME})
		install(TARGETS ${GAME_MODULE_NAME}
				RUNTIME DESTINATION ${INSTALL_DIR}/bin
				LIBRARY DESTINATION ${INSTALL_DIR}/bin
				PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
							GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
	endforeach()
endif()

if(UNIX AND NOT APPLE)
	configure_file("WeepEngine.cmake.desktop" "WeepEngine.desktop")
endif()

install(DIRECTORY data DESTINATION ${INSTALL_DIR})
install(DIRECTORY weep-media OPTIONAL DESTINATION ${INSTALL_DIR} PATTERN ".git" EXCLUDE)
