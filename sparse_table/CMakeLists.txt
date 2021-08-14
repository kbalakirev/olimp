cmake_minimum_required(VERSION 3.15)

project(CARTESIAN_TREE)

add_executable(sparse_table_example example.cpp)

target_include_directories(sparse_table_example PUBLIC ${ROOT_SOURCE_DIR})

set_property(TARGET sparse_table_example PROPERTY CXX_STANDARD 17)
