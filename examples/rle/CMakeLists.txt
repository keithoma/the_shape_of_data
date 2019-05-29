cmake_minimum_required(VERSION 2.8.11)
project(rle_save)

add_executable(rle_save main.cpp)
set_target_properties(rle_save PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(rle_save sgfx)
if (NOT MSVC)
	target_compile_options(rle_save PRIVATE -pedantic -Wall -Werror -Wno-error=attributes)
endif ()
