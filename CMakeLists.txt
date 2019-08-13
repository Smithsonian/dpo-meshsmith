cmake_minimum_required(VERSION 3.13.0)

# ------------------------------------------------------------------------------
# PROJECT

project(MeshSmith VERSION 0.7.0)

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
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/lib/debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/lib/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin/release)

# Append "d" to all debug libraries
set(CMAKE_DEBUG_POSTFIX "d")

# Visual Studio: include INSTALL target in default builds
set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD ON)

# GUI_TYPE can be added to add_executable to indicate a GUI application
# this prevents the console to be shown on windows
if(WIN32)
  set(GUI_TYPE WIN32)
endif()

if(APPLE)
  set(GUI_TYPE MACOSX_BUNDLE)
endif()

# ------------------------------------------------------------------------------
# COMPONENTS

add_subdirectory(libs/flow-libs libs/flow-libs/libs)

include_directories(libs/flow-libs/source)

add_subdirectory(source/core)
add_subdirectory(source/cli)
