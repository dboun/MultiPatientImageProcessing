cmake_minimum_required(VERSION 3.1.0)

project( MPIP )

find_package( MITK REQUIRED )

Include( mitkFunctionCheckCompilerFlags )

add_subdirectory( Applications )
# For the executable
mitk_create_executable( MPIP
  DEPENDS MitkQtWidgetsExt
  PACKAGE_DEPENDS ITK|ITKCurvatureFlow+ITKRegionGrowing
)
