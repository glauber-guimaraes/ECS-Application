cmake_minimum_required(VERSION 2.8)

project(entitypp)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")
add_library(entitypp STATIC ${SOURCES})

add_executable(example example/main.cpp)
target_link_libraries(example entitypp)
