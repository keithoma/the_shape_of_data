cmake_minimum_required(VERSION 2.8.11)

project(colorkey_example)

add_executable(colorkey_example main.cpp)
target_link_libraries(colorkey_example sgfx)
set_target_properties(colorkey_example PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
if(NOT MSVC)
	target_compile_options(colorkey_example PRIVATE -pedantic -Wall -Werror)
endif()
