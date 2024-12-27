cmake_minimum_required(VERSION 3.15)

# set the project name and version
project(VM VERSION 1.0)

set(CMAKE_DEBUG_POSTFIX d)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set_property(GLOBAL PROPERTY CMAKE_AUTO_REGEN TRUE)
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(vm src/main.cpp ${SOURCES})

target_include_directories(vm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

