find_package(benchmark REQUIRED)

add_executable(hw2_09_09 main.cpp)
target_compile_features(hw2_09_09 PRIVATE cxx_std_20)
target_compile_options(hw2_09_09 PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(hw2_09_09 PRIVATE benchmark::benchmark)
