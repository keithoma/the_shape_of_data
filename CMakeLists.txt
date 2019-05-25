cmake_minimum_required(VERSION 3.0)
project(TheShapeOfData)

cmake_policy(SET CMP0028 NEW)

option(SGFX_EXAMPLES "Build SGFX examples" ON)

add_subdirectory(sgfx)

if(SGFX_EXAMPLES)
#	add_subdirectory(examples/colorkey)
#	add_subdirectory(examples/huffman)
	add_subdirectory(examples/ppm)
#	add_subdirectory(examples/rle)
    add_subdirectory(examples/snake)
endif()