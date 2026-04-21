cmake_minimum_required(VERSION 3.14)

# Main project
project(start-lab VERSION 1.0.0 LANGUAGES C)

# This module helps with cloning and building other projects
include(FetchContent)

# Add an option for gradescope-specific build
option(BUILD_GRADESCOPE "Build the unit tests for gradescope" OFF)
option(BUILD_SOLUTIONS  "[INSTRUCTOR ONLY] Build with solutions for testing" OFF)
option(BUILD_BINUTILS "Add the binutils library to the workspace" OFF)

# grab the cgrade test library
FetchContent_Declare(
  cgrade
  GIT_REPOSITORY https://codeberg.org/mnoureddine/CGrade.git
  GIT_TAG main
)

# fetch the cgrade content
FetchContent_MakeAvailable(cgrade)

# Set some config options for output directories
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

# build everything with debug flags
add_compile_options(-Wall -Werror -ggdb)

# everything in src/ directory is relevant and should be built into libraries
add_subdirectory(src)

# check what compiler defitions to add
if(BUILD_GRADESCOPE)
  add_definitions(-DBUILD_GRADESCOPE)
endif(BUILD_GRADESCOPE)

if(BUILD_SOLUTIONS)
  MESSAGE(STATUS "WARNING: If you are a student and this shows up, you've messed up!")
endif(BUILD_SOLUTIONS)

if(BUILD_BINUTILS)
  MESSAGE(STATUS "LOG: Linking against binutils has been enabled!")
  add_definitions(-DBUILD_BINUTILS)
endif(BUILD_BINUTILS)

add_subdirectory(tests)

