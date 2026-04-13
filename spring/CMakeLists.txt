option(BUILD_HW1 "Build spring hw1 targets" ON)
option(BUILD_HW2 "Build spring hw2 targets" ON)

if(BUILD_HW1)
  add_subdirectory(hw1)
endif()

if(BUILD_HW2)
  add_subdirectory(hw2)
endif()
