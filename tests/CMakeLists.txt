# https://bas-man.dev/post/cmake/cplusplus-library-template/
# https://matgomes.com/integrate-google-test-into-cmake/
# https://raymii.org/s/tutorials/Cpp_project_setup_with_cmake_and_unit_tests.html
enable_testing()

file(GLOB SRC_FILES *.cpp)
add_executable(hello_test ${SRC_FILES} aggregator_test.cpp)

# add_executable(
#   hello_test
#   hello_test.cpp
#   factorial_test.cpp
# )
target_link_libraries(
  hello_test
  GTest::gtest_main
  wire
)

include(GoogleTest)
gtest_discover_tests(hello_test)
