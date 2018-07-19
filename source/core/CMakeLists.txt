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

set(Assimp_DIR "${PROJECT_SOURCE_DIR}/vendor/assimp")
set(Assimp_INCLUDE_DIR "${Assimp_DIR}/include")
set(Assimp_DEBUG_LIB "${Assimp_DIR}/lib/Debug/assimp-vc140-mt.lib")
set(Assimp_RELEASE_LIB "${Assimp_DIR}/lib/Release/assimp-vc140-mt.lib")
set(Assimp_DEBUG_DLL "${Assimp_DIR}/lib/Debug/assimp-vc140-mt.dll")
set(Assimp_RELEASE_DLL "${Assimp_DIR}/lib/Release/assimp-vc140-mt.dll")
message("Assimp IncludeDir: " ${Assimp_INCLUDE_DIR})

#set(FreeImage_INCLUDE_DIR "${FreeImage_DIR}")
#set(FreeImage_LIB "${FreeImage_DIR}/FreeImage.lib")
#set(FreeImage_DLL "${FreeImage_DIR}/FreeImage.dll")
#message("FreeImage Library: " ${FreeImage_LIB})
#message("FreeImage IncludeDir: " ${FreeImage_INCLUDE_DIR})

# Google Draco Mesh Compression
set(Draco_DIR "${PROJECT_SOURCE_DIR}/vendor/draco")
set(Draco_INCLUDE_DIR "${Draco_DIR}/include")
set(Draco_LIB_DEBUG "${Draco_DIR}/lib/Debug/draco.lib")
set(Draco_LIB_RELEASE "${Draco_DIR}/lib/Release/draco.lib")
message("Draco Directory: " ${Draco_DIR})

# ------------------------------------------------------------------------------
# BUILD TARGET

add_library(MeshSmithCore STATIC ${AllFiles})
add_definitions(-DMESHSMITH_CORE_LIB)
set_property(TARGET MeshSmithCore PROPERTY FOLDER "_libs")

target_include_directories(MeshSmithCore BEFORE PRIVATE
    FlowGLTF
	${Boost_INCLUDE_DIRS}
    ${Assimp_INCLUDE_DIR}
	${Draco_INCLUDE_DIR}
#    ${FreeImage_INCLUDE_DIR}
)

target_link_libraries(MeshSmithCore
	FlowGLTF
    debug ${Assimp_DEBUG_LIB}
	optimized ${Assimp_RELEASE_LIB}
	${Boost_LIBRARIES}
    optimized ${Draco_LIB_RELEASE}
	debug ${Draco_LIB_DEBUG}
#    ${FreeImage_LIB}
)

# ------------------------------------------------------------------------------
# INSTALL TARGET

# Copies the include headers into the top level include directory
#install(
#    FILES ${HeaderFiles}
#    DESTINATION ${CMAKE_SOURCE_DIR}/include/IntermeshEngine
#)

install(
    FILES  ${Assimp_DEBUG_DLL}
    DESTINATION ${CMAKE_SOURCE_DIR}/bin/Debug
)

install(
    FILES  ${Assimp_RELEASE_DLL}
    DESTINATION ${CMAKE_SOURCE_DIR}/bin/Release
)
