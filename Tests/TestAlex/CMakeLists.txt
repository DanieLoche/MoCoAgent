cmake_minimum_required(VERSION 2.8)
project(SchedulingManagement)

file(GLOB HEADERS include/*.hpp include/*.h)
file(GLOB SOURCES src/*.cpp)


set(SHARED_DIR "${CMAKE_SOURCE_DIR}/../shared")

set(BOOST_LATEST_DIR            "${SHARED_DIR}/boost_1_68_0")
set(BOOST_LATEST_BIN_DIR        "${BOOST_LATEST_DIR}/stage/lib")
set(BOOST_LATEST_INCLUDE_DIR    "${BOOST_LATEST_DIR}/boost")
set(BOOST_SYSTEM                "${BOOST_LATEST_BIN_DIR}/libboost_system.so")
set(BOOST_FS                    "${BOOST_LATEST_BIN_DIR}/libboost_filesystem.so")
set(BOOST_THREAD                "${BOOST_LATEST_BIN_DIR}/libboost_thread.so")


include_directories(${CMAKE_SOURCE_DIR} ${BOOST_LATEST_INCLUDE_DIR})

set(BOOST_LIBRARYDIR "/usr/lib/x86_64-linux-gnu/")
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)

find_package(Boost 1.65.0 REQUIRED COMPONENTS system thread filesystem)

add_executable(${PROJECT_NAME} "main.cpp" ${HEADERS} ${SOURCES})

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PUBLIC Boost::system Boost::filesystem Boost::thread)
endif()


