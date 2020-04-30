set(
  inc_dir
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(
  src_dir
  ${inc_dir}/lib
)

add_library(
  lib
)

target_compile_features(
  lib
  PRIVATE
    cxx_std_11
)

target_sources(
  lib
  PRIVATE
  "${src_dir}/lib.cpp"
  "${src_dir}/lib.hpp"
)

target_include_directories(
  lib
  PUBLIC
    ${inc_dir}
)

add_library(
  lib::lib
  ALIAS
  lib
)

add_executable(
  lib_test
)

target_sources(
  lib_test
  PRIVATE
  "${src_dir}/boost_fusion.test.cpp"
  "${src_dir}/cpp_17.test.cpp"
  "${src_dir}/main.test.cpp"
)

target_compile_features(
  lib_test
  PRIVATE
    cxx_std_11
)

target_link_libraries(
  lib_test
  PRIVATE
    Boost::unit_test_framework
)

add_test(
  NAME
    lib::test
  COMMAND
    lib_test
    "--color_output=false"
)
