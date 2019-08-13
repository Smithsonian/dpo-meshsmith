# ------------------------------------------------------------------------------
# MeshSmith - Core
# ------------------------------------------------------------------------------

# Automatically create a list of source files
file(GLOB SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Automatically create a list of header files
file(GLOB HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(AllFiles "${SourceFiles};${HeaderFiles}")
source_group("All Files" FILES ${AllFiles})

# ------------------------------------------------------------------------------
# 3RD PARTY LIBRARIES

# Assimp mesh importer/exporter library
set(Assimp_DIR "${PROJECT_SOURCE_DIR}/libs/assimp/bin")
set(Assimp_INCLUDE_DIR "${Assimp_DIR}/include")
set(Assimp_DEBUG_LIB "${Assimp_DIR}/lib/debug/assimp-vc142-mt.lib")
set(Assimp_RELEASE_LIB "${Assimp_DIR}/lib/release/assimp-vc142-mt.lib")
set(Assimp_DEBUG_DLL "${Assimp_DIR}/lib/debug/assimp-vc142-mt.dll")
set(Assimp_RELEASE_DLL "${Assimp_DIR}/lib/release/assimp-vc142-mt.dll")
message("Assimp Directory: " ${Assimp_DIR})

# Google Draco Mesh Compression
set(Draco_DIR "${PROJECT_SOURCE_DIR}/libs/draco/bin")
set(Draco_INCLUDE_DIR "${Draco_DIR}/include")
set(Draco_LIB_DEBUG "${Draco_DIR}/lib/debug/draco.lib")
set(Draco_LIB_RELEASE "${Draco_DIR}/lib/release/draco.lib")
message("Draco Directory: " ${Draco_DIR})

# ------------------------------------------------------------------------------
# BUILD TARGET

add_library(MeshSmithCore STATIC ${AllFiles})
add_definitions(-DMESHSMITH_CORE_LIB)
set_property(TARGET MeshSmithCore PROPERTY FOLDER "_libs")

target_include_directories(MeshSmithCore BEFORE PRIVATE
    FlowGLTF
    ${Assimp_INCLUDE_DIR}
	${Draco_INCLUDE_DIR}
)

target_link_libraries(MeshSmithCore
	FlowGLTF
    debug ${Assimp_DEBUG_LIB}
	optimized ${Assimp_RELEASE_LIB}
	debug ${Draco_LIB_DEBUG}
    optimized ${Draco_LIB_RELEASE}
)

# ------------------------------------------------------------------------------
# INSTALL TARGET

install(
    FILES  ${Assimp_DEBUG_DLL}
    DESTINATION ${CMAKE_SOURCE_DIR}/bin/debug
)

install(
    FILES  ${Assimp_RELEASE_DLL}
    DESTINATION ${CMAKE_SOURCE_DIR}/bin/release
)
