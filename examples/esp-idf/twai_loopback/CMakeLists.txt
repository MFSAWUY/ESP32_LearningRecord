cmake_minimum_required(VERSION 3.22)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# Build main and its explicitly declared dependency graph.
idf_build_set_property(MINIMAL_BUILD ON)
project(twai_loopback)
