cmake_minimum_required(VERSION 3.1.0)

project( MpipCore )

# C++11
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED YES)

# OpenMP is a feature of the compiler
FIND_PACKAGE(OpenMP REQUIRED)
SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )

# Base contains the data manager and interfaces for modules
add_subdirectory( base )

# Modules contains gui and algorithmic modules
add_subdirectory( modules )

# Create the MpipCore library
add_library( MpipCore INTERFACE )

target_link_libraries( MpipCore INTERFACE
  MpipBase
  MpipModules
)
