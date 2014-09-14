cmake_minimum_required (VERSION 2.6)
project (SSRAO)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin )
set( INCLUDE_INSTALL_DIR ${PROJECT_BINARY_DIR}/include )

# glm needs c++11
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x" )

add_subdirectory( cmake_files )
add_subdirectory( src )
