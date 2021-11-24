project(WeepEngine)
cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.1)

set(EXE_NAME "weep" CACHE STRING "Executable name")
set(INSTALL_DIR "weep" CACHE STRING "Installation directory name")
option(SHIPPING_BUILD "Remove debug stuff" OFF)
option(USE_REMOTERY "Use Remotery profiler" ON)
option(USE_GLES "Link against OpenGL ES" OFF)

# Avoid source tree pollution
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
endif()

# Add a sensible build type default and warning because empty means no optimization and no debug info.
if(NOT CMAKE_BUILD_TYPE)
	message("WARNING: CMAKE_BUILD_TYPE is not defined!\n         Defaulting to CMAKE_BUILD_TYPE=RelWithDebInfo.\n         Use ccmake to set a proper value.")
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# Enable C++11 support and other compiler flags...
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	message(STATUS "${CMAKE_CXX_COMPILER_ID} detected, adding compile flags")
	set(COMMON_FLAGS "-std=c++11 -ffast-math -march=native -Wall -Wextra")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}")
	if(WIN32)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--export-all-symbols")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	endif()
else()
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_CXX_STANDARD_REQUIRED YES)
endif()

if (DEFINED MSVC_VERSION)
	if (MSVC_VERSION GREATER_EQUAL 1500)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++11")
	endif()
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

if(WIN32)
	add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_CUSTOM="glad/glad.h")
else()
	add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_CUSTOM="GL/glcorearb.h")
endif()

if(USE_REMOTERY)
	add_definitions(-DUSE_PROFILER=1 -DRMT_USE_OPENGL=1 -DRMT_USE_POSIX_THREADNAMES=1)
	if(WIN32)
		list(APPEND LIBS "ws2_32")
	else()
		list(APPEND LIBS "dl")
	endif()
endif()

include_directories(engine)
include_directories(SYSTEM third-party)
include_directories(SYSTEM third-party/bullet)
include_directories(SYSTEM third-party/soloud/include)

if(SHIPPING_BUILD)
	add_definitions(-DSHIPPING_BUILD)
endif()
add_definitions(-DGL_GLEXT_PROTOTYPES)
# SoLoud
add_definitions(-DWITH_NULL -DWITH_SDL2_STATIC)

# TODO: Make separate libs of third-party deps

file(GLOB_RECURSE DEPS_SOURCES "third-party/*.cpp" "third-party/*.c" "third-party/*.hpp" "third-party/*.h")
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

add_library(deps STATIC ${DEPS_SOURCES})
add_library(engine STATIC ${ENGINE_SOURCES})
add_library(game STATIC ${GAME_SOURCES})

add_executable(${EXE_NAME} ${GAME_MAIN} ${SHADER_SOURCES} ${CONFIG_SOURCES})
set_target_properties(${EXE_NAME} PROPERTIES ENABLE_EXPORTS ON)
set_target_properties(${EXE_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON) # CMake 3.4
set_target_properties(${EXE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
target_link_libraries(${EXE_NAME} PRIVATE game engine deps ${LIBS})
install(TARGETS ${EXE_NAME} DESTINATION ${INSTALL_DIR}/bin)

# Game module plugins
foreach(GAME_MODULE_FILE ${MODULES_SOURCES})
	get_filename_component(GAME_MODULE_NAME ${GAME_MODULE_FILE} NAME_WE)
	add_library(${GAME_MODULE_NAME} MODULE ${GAME_MODULE_FILE})
	if(WIN32)
		# TODO This is very suboptimal...
		target_link_libraries(${GAME_MODULE_NAME} PRIVATE game engine deps ${LIBS})
	else()
		target_link_libraries(${GAME_MODULE_NAME} ${EXE_NAME})
	endif()
	install(TARGETS ${GAME_MODULE_NAME}
			RUNTIME DESTINATION ${INSTALL_DIR}/bin
			LIBRARY DESTINATION ${INSTALL_DIR}/bin
			PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
						GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
endforeach()


if(UNIX AND NOT APPLE)
	configure_file("WeepEngine.cmake.desktop" "WeepEngine.desktop")
endif()

install(DIRECTORY data DESTINATION ${INSTALL_DIR})
install(DIRECTORY weep-media OPTIONAL DESTINATION ${INSTALL_DIR} PATTERN ".git" EXCLUDE)
