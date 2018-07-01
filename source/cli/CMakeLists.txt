# ------------------------------------------------------------------------------
# MeshSmith - CLI
# ------------------------------------------------------------------------------

# Automatically create a list of source files
file(GLOB SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Automatically create a list of header files
file(GLOB HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(AllFiles ${SourceFiles};${HeaderFiles})
source_group("All Files" FILES ${AllFiles})

# ------------------------------------------------------------------------------
# 3RD PARTY LIBRARIES

#find_package(Qt5 COMPONENTS Core REQUIRED)

# ------------------------------------------------------------------------------
# BUILD TARGET

add_executable(MeshSmith ${AllFiles})
set_target_properties(MeshSmith PROPERTIES DEBUG_POSTFIX "d")
set_property(TARGET MeshSmith PROPERTY FOLDER "_apps")

target_include_directories(MeshSmith BEFORE PRIVATE
    ${PROJECT_SOURCE_DIR}/source
)
#Qt5::Core

target_link_libraries(MeshSmith
    FlowGLTF
    MeshSmithCore
)
#Qt5::Core

# ------------------------------------------------------------------------------
# INSTALL TARGET

#install(
#    FILES  ${Assimp_DLL} ${FreeImage_DLL}
#    DESTINATION ${CMAKE_SOURCE_DIR}/bin
#)
