cmake_minimum_required(VERSION 2.8.11)
project(snake_example)

if (NOT MSVC)
	add_definitions("-pedantic")
	add_definitions("-Wall")
	add_definitions("-Werror")
	add_definitions("-Wno-error=attributes")
endif()

add_executable(snake_example main.cpp)
set_target_properties(snake_example PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(snake_example sgfx)
