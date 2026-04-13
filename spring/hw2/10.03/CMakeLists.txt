find_package(Boost REQUIRED)

add_executable(hw2_10_03 main.cpp)
target_compile_features(hw2_10_03 PRIVATE cxx_std_20)
target_compile_options(hw2_10_03 PRIVATE -Wall -Wextra -Wpedantic)

if(TARGET Boost::headers)
  target_link_libraries(hw2_10_03 PRIVATE Boost::headers)
elseif(TARGET Boost::boost)
  target_link_libraries(hw2_10_03 PRIVATE Boost::boost)
endif()
