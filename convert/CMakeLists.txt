cmake_minimum_required(VERSION 2.8.11)
project(convert CXX)

include(CheckIncludeFiles)
include(CheckIncludeFileCXX)
include(CheckFunctionExists)

CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILES(ioctl.h HAVE_IOCTL_H)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sysconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/sysconfig.h)

add_executable(convert
	main.cpp
	flags.cpp
	huffman.cpp
	pipeline.cpp
)

set_target_properties(convert PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

target_include_directories(convert PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(convert sgfx) # actually BS, as we only need <primitive_types.hpp>?

if (NOT MSVC)
	target_compile_definitions(convert PRIVATE "-pedantic" "-Wall" "-Werror" "Wno-error=attributes")
endif ()
