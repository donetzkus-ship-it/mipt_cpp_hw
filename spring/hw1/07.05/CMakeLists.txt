find_package(benchmark REQUIRED)

add_executable(hw1_07_05_bench main.cpp)

target_compile_features(hw1_07_05_bench PRIVATE cxx_std_20)
target_compile_options(hw1_07_05_bench PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(hw1_07_05_bench PRIVATE benchmark::benchmark)
