cmake_minimum_required(VERSION 3.1.0)

project(Viz2)

find_package(MITK REQUIRED)

Include(mitkFunctionCheckCompilerFlags)

mitk_create_executable(viz2
  DEPENDS MitkQtWidgetsExt
  PACKAGE_DEPENDS ITK|ITKCurvatureFlow+ITKRegionGrowing
  )
