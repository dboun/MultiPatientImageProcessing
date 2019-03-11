project( ${MODULE_NAME} )

find_package( Qt5 COMPONENTS Widgets REQUIRED )
qt5_use_modules( MpipCore Core Gui Widgets )

qt5_wrap_ui( UI_Srcs ${UI_FILES} )

add_library( ${MODULE_NAME} 
  ${CPP_FILES} ${H_FILES} ${UI_FILES} ${MOC_H_FILES} 
)

target_link_libraries( ${MODULE_NAME} 
  MpipBase
)

target_include_directories( ${MODULE_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)