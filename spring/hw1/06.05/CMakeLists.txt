set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost REQUIRED COMPONENTS filesystem system)

add_library(hw1_06_05_v1 SHARED lib_v1.cpp)
add_library(hw1_06_05_v2 SHARED lib_v2.cpp)

add_executable(hw1_06_05 main.cpp)

target_compile_features(hw1_06_05_v1 PRIVATE cxx_std_20)
target_compile_features(hw1_06_05_v2 PRIVATE cxx_std_20)
target_compile_features(hw1_06_05 PRIVATE cxx_std_20)

target_compile_options(hw1_06_05_v1 PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(hw1_06_05_v2 PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(hw1_06_05 PRIVATE -Wall -Wextra -Wpedantic)

if(TARGET Boost::headers)
    target_link_libraries(hw1_06_05 PRIVATE Boost::headers)
elseif(TARGET Boost::boost)
    target_link_libraries(hw1_06_05 PRIVATE Boost::boost)
endif()

if(TARGET Boost::filesystem)
    target_link_libraries(hw1_06_05 PRIVATE Boost::filesystem)
endif()

if(TARGET Boost::system)
    target_link_libraries(hw1_06_05 PRIVATE Boost::system)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(hw1_06_05 PRIVATE dl)
endif()

set_target_properties(hw1_06_05_v1 hw1_06_05_v2 PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

set_target_properties(hw1_06_05 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
