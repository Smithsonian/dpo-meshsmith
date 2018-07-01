cmake_minimum_required(VERSION 3.9.0)

# ------------------------------------------------------------------------------
# PROJECT

project(MeshSmith VERSION 0.6)

# ------------------------------------------------------------------------------
# GLOBAL SETTINGS

#Compiler flags
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -Wall")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -msse4")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
#    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W2")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# Output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# Append "d" to all debug libraries
set(CMAKE_DEBUG_POSTFIX "d")

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

# GUI_TYPE can be added to add_executable to indicate a GUI application
# this prevents the console to be shown on windows
if(WIN32)
  set(GUI_TYPE WIN32)
endif()

if(APPLE)
  set(GUI_TYPE MACOSX_BUNDLE)
endif()

# ------------------------------------------------------------------------------
# 3RD PARTY LIBRARIES

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_DEBUG_RUNTIME OFF)
find_package(Boost REQUIRED filesystem)
message(STATUS "Boost include directories: " ${Boost_INCLUDE_DIRS})
message(STATUS "Boost library directories: " ${Boost_LIBRARY_DIRS})

# ------------------------------------------------------------------------------
# COMPONENTS

# Add base include path for all libraries
#include_directories (${PROJECT_SOURCE_DIR}/source)

add_subdirectory(../flow-libs ${PROJECT_SOURCE_DIR}/libs/flow-lib)
include_directories(../flow-libs/source)

add_subdirectory(source/core)
add_subdirectory(source/cli)
