set(
  inc_dir
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(
  src_dir
  ${inc_dir}/lib
)

add_library(
  tuple
)

target_compile_features(
  tuple
  PRIVATE
    cxx_std_11
)

target_sources(
  tuple
  PRIVATE
    "${src_dir}/tuple.cpp"
    "${src_dir}/tuple.hpp"
)

target_include_directories(
  tuple
  PUBLIC
    ${inc_dir}
)

target_link_libraries(
  tuple
  PUBLIC
    Boost::boost
)

add_library(
  lib::tuple
  ALIAS
    tuple
)

add_executable(
  tuple_test
)

target_sources(
  tuple_test
  PRIVATE
    "${src_dir}/boost_fusion.test.cpp"
    "${src_dir}/cpp_17.test.cpp"
    "${src_dir}/main.test.cpp"
)

target_compile_features(
  tuple_test
  PRIVATE
    cxx_std_17
)

target_link_libraries(
  tuple_test
  PRIVATE
    lib::tuple
    Boost::boost
    Boost::unit_test_framework
)

add_test(
  NAME
    lib::tuple_test
  COMMAND
    tuple_test
    "--color_output=false"
)
