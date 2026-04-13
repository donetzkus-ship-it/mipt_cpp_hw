include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-m32" HW2_10_05_HAS_M32)

add_executable(hw2_10_05 main.cpp)
target_compile_features(hw2_10_05 PRIVATE cxx_std_20)
target_compile_options(hw2_10_05 PRIVATE -Wall -Wextra -Wpedantic -O3)

if(HW2_10_05_HAS_M32)
  target_compile_options(hw2_10_05 PRIVATE -m32)
  target_link_options(hw2_10_05 PRIVATE -m32)
endif()
