CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(MODS ${CMAKE_SOURCE_DIR}/cmake/)
SET(CMAKE_MODULE_PATH ${MODS} ${CMAKE_MODULE_PATH})

FIND_PACKAGE(Erlang)

PROJECT(erlconnect)

INCLUDE_DIRECTORIES(
        ${ERLANG_EI_INCLUDE_PATH}
        ${ERLANG_ERTS_INCLUDE_PATH}
)

LINK_DIRECTORIES(
        ${ERLANG_EI_LIBRARY_PATH}
)


ADD_EXECUTABLE(erlconnect erlconnect.cpp)
TARGET_LINK_LIBRARIES(erlconnect erl_interface ei pthread)
