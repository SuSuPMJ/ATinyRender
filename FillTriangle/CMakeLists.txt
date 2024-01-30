cmake_minimum_required(VERSION 3.15)
project(Test)
aux_source_directory(${PROJECT_SOURCE_DIR} Source)
add_executable(app ${Source})
