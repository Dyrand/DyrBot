cmake_minimum_required (VERSION 2.8)
project (DyrBot)

include_directories("${PROJECT_BINARY_DIR}")

SET(GCC_COVERAGE_COMPILE_FLAGS "-std=c++11")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

#Find Boost
find_package(Boost REQUIRED COMPONENTS system thread filesystem boost_program_options)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_executable(
  DyrBot
  src/main.cpp
  src/ConnectionManager.cpp
  src/DyrBot.cpp
  src/parsers/message_parser.cpp
  src/parsers/privmsg_parse.cpp
  src/error/error_logger.cpp
  src/modules/module.cpp
  src/modules/custodian/commands.cpp
)

target_link_libraries(DyrBot ${Boost_LIBRARIES})

if(WIN32)
  target_link_libraries(DyrBot wsock32 ws2_32)
endif()
