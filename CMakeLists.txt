
project(st-tilemanager)
cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
add_executable(st-tilemanager ${SOURCE_FILES})

set(HARBOR_BUILD_EXEC OFF)
set(HARBOR_BUILD_TEST OFF)
set(HARBOR_USE_SCRIPTING OFF)
set(HARBOR_USE_VIDEO_OPENGL OFF)
add_subdirectory(external/harbor)

target_link_libraries(st-tilemanager PUBLIC harbor_lib)
target_include_directories(st-tilemanager PUBLIC external/harbor/src
                                                 external/portable-file-dialogs
                                                 src)
