cmake_minimum_required (VERSION 2.8.3)

# enable IDE project folders
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

project(PXLJM14 CXX C)

#option(BUILD_DOCUMENTATION "Use Doxygen to generate HTML based API documentation" OFF)
#if(BUILD_DOCUMENTATION)
#	FIND_PACKAGE(Doxygen)
#	if(NOT DOXYGEN_FOUND)
#		message(FATAL_ERROR)

# OpenGL
include(FindOpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

#Spine-C
add_subdirectory("${PROJECT_SOURCE_DIR}/ext/spine-c" "${PROJECT_BINARY_DIR}/ext/spinec")
include_directories("${PROJECT_SOURCE_DIR}/ext/spine-c/include")

# GLFW
add_subdirectory("${PROJECT_SOURCE_DIR}/ext/glfw-3.0.3" "${PROJECT_BINARY_DIR}/ext/glfw")
include_directories("${PROJECT_SOURCE_DIR}/ext/glfw-3.0.3/include")

# GLEW
add_subdirectory("${PROJECT_SOURCE_DIR}/ext/glew-1.10.0" "${PROJECT_BINARY_DIR}/ext/glew")
include_directories("${PROJECT_SOURCE_DIR}/ext/glew-1.10.0/include")

# Box2D
add_subdirectory("${PROJECT_SOURCE_DIR}/ext/box2d-2.3.1/Box2D" "${PROJECT_BINARY_DIR}/ext/box2d")
include_directories("${PROJECT_SOURCE_DIR}/ext/box2d-2.3.1/Box2D")

# CRYPTOPP
# turned off because its not friendly in VS atm and we dont need it yet either
#add_subdirectory("${PROJECT_SOURCE_DIR}/ext/cryptopp-5.6.2" "${PROJECT_BINARY_DIR}/ext/cryptopp")
#include_directories("${PROJECT_SOURCE_DIR}/ext/cryptopp-5.6.2/src")

# GTEST
add_subdirectory("${PROJECT_SOURCE_DIR}/ext/gtest" "${PROJECT_BINARY_DIR}/ext/gtest")


# compiler flags 
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Werror=return-type -pedantic -std=c++11 -g")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Werror=return-type -pedantic -std=c++11 -g")
endif()

# output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# source
add_subdirectory(src)

# resources - this is a custom target with no build step;
# primarily to allow shader editing in IDEs
add_subdirectory(res)

# set IDE display folders
SET_PROPERTY(TARGET glfw PROPERTY FOLDER "ext")
SET_PROPERTY(TARGET glew PROPERTY FOLDER "ext")
SET_PROPERTY(TARGET gtest PROPERTY FOLDER "ext")
SET_PROPERTY(TARGET gtest_main PROPERTY FOLDER "ext")
#SET_PROPERTY(TARGET cryptopp PROPERTY FOLDER "ext")
SET_PROPERTY(TARGET Box2D PROPERTY FOLDER "ext")
SET_PROPERTY(TARGET spinec PROPERTY FOLDER "ext")
SET_PROPERTY(TARGET gecom PROPERTY FOLDER "src")
SET_PROPERTY(TARGET pxljm PROPERTY FOLDER "src")
SET_PROPERTY(TARGET res PROPERTY FOLDER "res")





















