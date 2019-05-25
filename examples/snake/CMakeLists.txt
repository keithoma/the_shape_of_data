cmake_minimum_required(VERSION 2.8.11)
project(snake_example)

add_executable(snake_example main.cpp)
set_target_properties(snake_example PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(snake_example sgfx)
if (NOT MSVC)
target_compile_definitions(snake_example PRIVATE -pedantic -Wall -Werror -Wno-error=attributes)
endif()
