set( ssrao_SRCS main.cpp
                GLObject.cpp
                window.cpp )
                
add_executable( ssrao ${ssrao_SRCS} )

target_link_libraries( ssrao GLEW GL GLU X11 assimp )
