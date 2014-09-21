cmake_minimum_required (VERSION 2.6)
project (SSRAO)

if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  include_directories( "/opt/X11/include" "/opt/local/incude" )
  link_directories( "/opt/X11/lib" "/opt/local/lib" )
endif()

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin )
set( INCLUDE_INSTALL_DIR ${PROJECT_BINARY_DIR}/include )

# glm needs c++11
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x" )

set( WITH_IL 1 )
#set( WITH_ILUT 1 )
set( SSRAO_BASE_PATH ${CMAKE_SOURCE_DIR} )

add_subdirectory( cmake_files )
add_subdirectory( src )
