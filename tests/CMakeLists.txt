cmake_minimum_required(VERSION 3.12)

# -----------------------------------------------------------------------------
# 3ed Party
# -----------------------------------------------------------------------------
# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
  )
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# -----------------------------------------------------------------------------
# tests
# -----------------------------------------------------------------------------

add_executable(test_handy_variant
  test_handy_variant_001.cpp
)

target_include_directories(test_handy_variant PRIVATE
  ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(test_handy_variant
  handy_variant
  gtest
  gtest_main
)

gtest_discover_tests(
  test_handy_variant
  XML_OUTPUT_DIR unittest_result
)