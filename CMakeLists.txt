project(SDLTest)
cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

# Avoid source tree pollution
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
endif()

# Add a sensible build type default and warning because empty means no optimization and no debug info.
if(NOT CMAKE_BUILD_TYPE)
	message("WARNING: CMAKE_BUILD_TYPE is not defined!\n         Defaulting to CMAKE_BUILD_TYPE=RelWithDebInfo.\n         Use ccmake to set a proper value.")
	SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# Enable C++11 support and other compiler flags...
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	message(STATUS "GCC detected, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11 -Wall -Wextra")
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	message(STATUS "Clang detected, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
endif()

find_package(PkgConfig)

pkg_check_modules(SDL2 REQUIRED sdl2)
include_directories(${SDL2_INCLUDE_DIRS})
list(APPEND LIBS ${SDL2_LIBRARIES})

pkg_check_modules(GL REQUIRED gl)
list(APPEND LIBS ${GL_LIBRARIES})

include_directories(engine)
include_directories(SYSTEM third-party)
include_directories(SYSTEM third-party/bullet)

add_definitions(-DGL_GLEXT_PROTOTYPES)

file(GLOB_RECURSE DEPS_SOURCES "third-party/*.cpp" "third-party/*.hpp" "third-party/*.h")
file(GLOB_RECURSE ENGINE_SOURCES "engine/*.cpp" "engine/*.hpp")
file(GLOB_RECURSE GAME_SOURCES "game/*.cpp" "game/*.hpp")
file(GLOB_RECURSE SHADER_SOURCES "data/shaders/*.*")
file(GLOB_RECURSE CONFIG_SOURCES "data/*.json")
set_source_files_properties(${SHADER_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(${CONFIG_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
link_directories(${PROJECT_BINARY_DIR})

add_library(deps STATIC ${DEPS_SOURCES})
add_library(engine STATIC ${ENGINE_SOURCES})
add_executable(app ${GAME_SOURCES} ${SHADER_SOURCES} ${CONFIG_SOURCES})
target_link_libraries(app engine deps ${LIBS})
