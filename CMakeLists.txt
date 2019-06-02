cmake_minimum_required(VERSION 3.0)
project(TheShapeOfData)

cmake_policy(SET CMP0028 NEW)

option(SGFX_EXAMPLES "Build SGFX examples" ON)

find_program(
	CLANG_TIDY_EXE
	NAMES "clang-tidy"
	DOC "Path to clang-tidy executable"
)
if(NOT CLANG_TIDY_EXE)
	message(STATUS "clang-tidy not found.")
else()
	message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
	set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

add_subdirectory(sgfx)
add_subdirectory(convert)

if(SGFX_EXAMPLES)
	add_subdirectory(examples/colorkey)
#	add_subdirectory(examples/huffman)
	add_subdirectory(examples/ppm)
	add_subdirectory(examples/rle)
    add_subdirectory(examples/snake)
endif()
