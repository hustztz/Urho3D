cmake_minimum_required( VERSION 2.7 )

project( GIS )

file( GLOB GIS_HEADER_FILES *.hpp *.h )
file( GLOB GIS_SOURCE_FILES *.cpp )

add_executable( GIS ${GIS_HEADER_FILES} ${GIS_SOURCE_FILES} )

target_link_libraries( GIS PRIVATE Vking-Engine-Core )

target_compile_options( GIS PRIVATE "/std:c++17" )

target_include_directories( GIS PRIVATE 
                ${CMAKE_BINARY_DIR}/include
                ${CMAKE_BINARY_DIR}/include/Urho3D/ThirdParty
                ${CMAKE_BINARY_DIR}/include/Urho3D/ThirdParty/Bullet
                ${CMAKE_BINARY_DIR}/include/Urho3D/ThirdParty/Lua
                ${CMAKE_BINARY_DIR}/include/Urho3D/ThirdParty/gtest
                ${CMAKE_SOURCE_DIR}/Source/Samples
                ${CMAKE_SOURCE_DIR}/Source/Tools
                ${CMAKE_SOURCE_DIR}/Source/ThirdParty/Assimp/include )