cmake_minimum_required(VERSION 2.8.11)

include(CheckCXXCompilerFlag)

project(libsgfx)

find_package(glfw3)
find_package(GLEW)
find_package(OpenGL)

if(MSVC)
	add_definitions(-DNOMINMAX)
else()
	add_definitions(-Wall)
	add_definitions(-pedantic)
	add_definitions(-Werror)
endif(MSVC)

add_library(sgfx STATIC
	src/canvas.cpp
	src/image.cpp
	src/ppm.cpp
	src/primitives.cpp
	src/window.cpp
)
set_target_properties(sgfx PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_include_directories(sgfx PUBLIC include)

set(libs glfw GLEW::GLEW OpenGL::GL)
if (NOT MSVC)
	set(libs ${libs} stdc++fs)
endif()
target_link_libraries(sgfx ${libs})

install(TARGETS sgfx DESTINATION lib)
install(DIRECTORY include/sgfx DESTINATION include)
