cmake_minimum_required(VERSION 3.1)

project(TransactionManager)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig)
pkg_check_modules(FIND_PKG REQUIRED glib-2.0 jsoncpp libcurl)

include_directories(/usr/include ${FIND_PKG_INCLUDE_DIRS})

file(GLOB sources src/*.cpp)

include(CMakePrintHelpers)

add_executable(${PROJECT_NAME} ${sources})

target_include_directories(${PROJECT_NAME} PRIVATE ./include)
target_link_libraries(${PROJECT_NAME} binance-cxx-api ${FIND_PKG_LIBRARIES})
