set( ssrao_SRCS main.cpp
                GLObject.cpp )
              
include_directories( ${INCLUDE_INSTALL_DIR} )
 
add_executable( ssrao ${ssrao_SRCS} )

target_link_libraries( ssrao GLEW GL GLU glut assimp )
