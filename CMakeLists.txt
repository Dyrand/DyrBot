cmake_minimum_required (VERSION 2.8)
project (DyrBot)

include_directories("${PROJECT_BINARY_DIR}/include")

SET(GCC_COVERAGE_COMPILE_FLAGS "-std=c++14")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

#Find Boost
find_package(Boost REQUIRED COMPONENTS system thread filesystem)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_executable(
  DyrBot
  src/main.cpp
)

target_link_libraries(DyrBot ${Boost_LIBRARIES})

if(WIN32)
  target_link_libraries(DyrBot wsock32 ws2_32)
endif()
