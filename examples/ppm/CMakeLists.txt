cmake_minimum_required(VERSION 2.8.11)
project(ppm_example)

if(NOT MSVC)
	add_definitions(-Wall)
	add_definitions(-pedantic)
	add_definitions(-Werror)
endif()

add_executable(sgfx_ppm main.cpp)
set_target_properties(sgfx_ppm PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(sgfx_ppm sgfx)