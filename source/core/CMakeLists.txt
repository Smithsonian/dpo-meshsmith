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
set(Assimp_DEBUG_LIB "${Assimp_DIR}/lib/debug/assimp-vc140-mt.lib")
set(Assimp_RELEASE_LIB "${Assimp_DIR}/lib/release/assimp-vc140-mt.lib")
set(Assimp_DEBUG_DLL "${Assimp_DIR}/lib/debug/assimp-vc140-mt.dll")
set(Assimp_RELEASE_DLL "${Assimp_DIR}/lib/release/assimp-vc140-mt.dll")
message("Assimp IncludeDir: " ${Assimp_INCLUDE_DIR})

#set(FreeImage_INCLUDE_DIR "${FreeImage_DIR}")
#set(FreeImage_LIB "${FreeImage_DIR}/FreeImage.lib")
#set(FreeImage_DLL "${FreeImage_DIR}/FreeImage.dll")
#message("FreeImage Library: " ${FreeImage_LIB})
#message("FreeImage IncludeDir: " ${FreeImage_INCLUDE_DIR})

# ------------------------------------------------------------------------------
# BUILD TARGET

add_library(MeshSmithCore STATIC ${AllFiles})
add_definitions(-DMESHSMITH_CORE_LIB)
set_property(TARGET MeshSmithCore PROPERTY FOLDER "_libs")

target_include_directories(MeshSmithCore BEFORE PRIVATE
    FlowGLTF
	${Boost_INCLUDE_DIRS}
    ${Assimp_INCLUDE_DIR}
#    ${FreeImage_INCLUDE_DIR}
)

target_link_libraries(MeshSmithCore
    debug ${Assimp_DEBUG_LIB}
	optimized ${Assimp_RELEASE_LIB}
	${Boost_LIBRARIES}
    FlowGLTF
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
    DESTINATION ${CMAKE_SOURCE_DIR}/bin/debug
)

install(
    FILES  ${Assimp_RELEASE_DLL}
    DESTINATION ${CMAKE_SOURCE_DIR}/bin/release
)
