set( GLM_FORCE_RADIANS 1 )

configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
                "${INCLUDE_INSTALL_DIR}/config.h" )
