cmake_minimum_required(VERSION 3.15)

project(CARTESIAN_TREE)

set(CMAKE_CXX_STANDARD 20)

add_executable(segment_tree_example example.cpp)

target_include_directories(segment_tree_example PUBLIC ${ROOT_SOURCE_DIR})

set_property(TARGET segment_tree_example PROPERTY CXX_STANDARD 17)
