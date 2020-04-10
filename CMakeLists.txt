cmake_minimum_required(
  VERSION
    3.15
)

project(
  project_name
  VERSION
    0.0
  DESCRIPTION
    "enter description here"
)

enable_testing()

set(
  CMAKE_EXPORT_COMPILE_COMMANDS
  ON
)

add_subdirectory(
  app
)

add_subdirectory(
  lib
)

add_subdirectory(
  external
)
