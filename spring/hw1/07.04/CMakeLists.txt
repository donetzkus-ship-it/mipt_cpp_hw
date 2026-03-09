find_package(GTest REQUIRED)

enable_testing()

add_executable(hw1_07_04_tests main.cpp)

target_compile_features(hw1_07_04_tests PRIVATE cxx_std_20)
target_compile_options(hw1_07_04_tests PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(hw1_07_04_tests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(hw1_07_04_tests)
