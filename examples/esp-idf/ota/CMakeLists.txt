cmake_minimum_required(VERSION 3.22)
set(PROJECT_VER "1.0.0")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# Build main and its explicitly declared dependency graph.
idf_build_set_property(MINIMAL_BUILD ON)
project(ota)
