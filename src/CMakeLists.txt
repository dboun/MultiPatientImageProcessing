include( cmake/dependencies.cmake )
include( cmake/files.cmake )

set( CMAKE_AUTOMOC ON )

qt5_wrap_ui( UI_Srcs ${UI_FILES} )

# Create the executable
add_executable( ${PROJECT_NAME} 
  ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES} 
)

target_link_libraries( ${PROJECT_NAME} 
  MpipCore
)

target_include_directories( ${PROJECT_NAME}
  ${CURRENT_LIST_DIR}
)
