cmake_minimum_required (VERSION 2.8)
project (DyrBot)

include_directories("${PROJECT_SOURCE_DIR}/include")

SET(COVERAGE_COMPILE_FLAGS "-std=c++11 -Wall")
SET(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -DCONSOLE_OUT")

if(CMAKE_BUILD_TYPE EQUAL DEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE EQUAL RELEASE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
endif(CMAKE_BUILD_TYPE EQUAL DEBUG)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILE_FLAGS}" )

#Find Boost
find_package(Boost REQUIRED COMPONENTS system thread filesystem)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_executable(
  DyrBot
  src/main.cpp
  src/UUID/UniqueIdentifier.cpp
  src/Logger/Logger.cpp
  src/BotManager/BotManager.cpp
  src/DyrBot/DyrBot.cpp
  src/ConnectionManager/ConnectionManager.cpp
  src/Parser/Parser.cpp
)

target_link_libraries(DyrBot ${Boost_LIBRARIES})

if(WIN32)
  target_link_libraries(DyrBot wsock32 ws2_32)
else
  target_link_libraries(DyrBot pthread)
endif(WIN32Z)
