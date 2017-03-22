CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

set(CMAKE_CXX_STANDARD 11)

SET(MODS ${CMAKE_SOURCE_DIR}/cmake/)
SET(CMAKE_MODULE_PATH ${MODS} ${CMAKE_MODULE_PATH})

FIND_PACKAGE(Erlang)

PROJECT(erlang_c_cnode)
file(GLOB_RECURSE so_sources  src/*.cpp src/*.h)
file(GLOB_RECURSE test_sources  test/*.cpp test/*.h)


INCLUDE_DIRECTORIES(
        ${ERLANG_EI_INCLUDE_PATH}
        ${ERLANG_ERTS_INCLUDE_PATH}
        src
)

LINK_DIRECTORIES(
        ${ERLANG_EI_LIBRARY_PATH}
)

ADD_LIBRARY(erlconnect SHARED ${so_sources})
ADD_EXECUTABLE(cnodetest ${test_sources})

target_compile_options(cnodetest PUBLIC -std=c++11 -Wall)
target_compile_options(erlconnect PUBLIC -std=c++11 -Wall)

TARGET_LINK_LIBRARIES(erlconnect erl_interface ei pthread)
TARGET_LINK_LIBRARIES(cnodetest erlconnect)