cmake_minimum_required(VERSION 3.10)

# set the project name
project(tinyECS VERSION 1.0 LANGUAGES CXX)

# Fetch Embedded Template Library for development (auto-download)
include(FetchContent)

FetchContent_Declare(
	etl
	GIT_REPOSITORY https://github.com/ETLCPP/etl
	GIT_TAG        20.44.2
)

FetchContent_MakeAvailable(etl)

add_library(${PROJECT_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/src/tiny_ecs.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE etl::etl)

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	EXPORT_COMPILE_COMMANDS ON
)

target_compile_options(${PROJECT_NAME} PRIVATE -nostdlib)

# add the executable
add_executable(ecs_demo ${CMAKE_CURRENT_SOURCE_DIR}/src/ecs_demo.cpp)
target_link_libraries(ecs_demo PRIVATE tinyECS etl::etl)

set_target_properties(ecs_demo PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	EXPORT_COMPILE_COMMANDS ON
)

install(TARGETS ${PROJECT_NAME}
	DESTINATION lib
)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/tiny_ecs.hpp
	DESTINATION include
)
