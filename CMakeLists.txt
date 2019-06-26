cmake_minimum_required(VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

project(entitypp)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")
add_library(entitypp STATIC ${SOURCES})

add_executable(example example/main.cpp)
target_link_libraries(example entitypp)
