cmake_minimum_required(VERSION 3.24)
project(Solar_System_Simulation)

set(CMAKE_CXX_STANDARD 17)

add_executable(Solar_System_Simulation main.cpp)
target_link_libraries(Solar_System_Simulation -lglew32 -lopengl32 -lfreeglut -lglu32)

