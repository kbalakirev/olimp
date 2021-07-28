cmake_minimum_required(VERSION 3.15)

project(ALLOC)

set(CMAKE_CXX_STANDARD 20)

add_executable(alloc_example example.cpp)

target_include_directories(alloc_example PUBLIC ${ROOT_SOURCE_DIR})

set_property(TARGET alloc_example PROPERTY CXX_STANDARD 20)
