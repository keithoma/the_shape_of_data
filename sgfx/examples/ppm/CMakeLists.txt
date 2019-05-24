if (NOT MSVC)
	add_definitions(-Wall)
	add_definitions(-pedantic)
	add_definitions(-Werror)
endif()

add_executable(ppm_example
	main.cpp
)
set_target_properties(ppm_example PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(ppm_example sgfx)
install(TARGETS ppm_example DESTINATION bin)