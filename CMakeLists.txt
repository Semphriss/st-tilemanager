project(st-tilemanager)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/supertux/*.cpp src/supertux/util/*.cpp)
add_executable(st-tilemanager ${SOURCE_FILES})

set(HARBOR_BUILD_EXEC OFF)
set(HARBOR_BUILD_TEST OFF)
set(HARBOR_USE_SCRIPTING OFF)
set(HARBOR_USE_VIDEO_OPENGL OFF)
add_subdirectory(external/harbor)

include(ProvideSexpcpp)

target_link_libraries(st-tilemanager PUBLIC harbor_lib)
target_link_libraries(st-tilemanager PUBLIC LibSexp)
target_include_directories(st-tilemanager PUBLIC external/harbor/src
                                                 external/portable-file-dialogs
                                                 src)
