cmake_minimum_required(VERSION 2.6)

project(ham C CXX)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

# only build examples if this is the main project
if(CMAKE_PROJECT_NAME STREQUAL "ham")
  option(HAM_BUILD_TESTS "Build Ham examples" ON)
endif()

set(CXX_FLAGS
  -g
  # -DVALGRIND
  -DCHECK_PTHREAD_RETURN_VALUE
  -D_FILE_OFFSET_BITS=64
  -Wall
  -Wextra
  -Werror
  -Wconversion
  -Wno-unused-parameter
  -Wno-unused-variable
  -Wno-error=unused-result
  -Wold-style-cast
  -Woverloaded-virtual
  -Wpointer-arith
  -Wshadow
  -Wwrite-strings
  -march=native
  # -MMD
  -std=c++11
  -rdynamic
  -pthread   # check
 )
if(CMAKE_BUILD_BITS EQUAL 32)
  list(APPEND CXX_FLAGS "-m32")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  list(APPEND CXX_FLAGS "-Wno-null-dereference")
  list(APPEND CXX_FLAGS "-Wno-sign-conversion")
  list(APPEND CXX_FLAGS "-Wno-unused-local-typedef")
  list(APPEND CXX_FLAGS "-Wthread-safety")
  list(REMOVE_ITEM CXX_FLAGS "-rdynamic")
endif()


string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

find_package(Boost REQUIRED)
find_package(CURL)
find_path(CARES_INCLUDE_DIR ares.h)
find_library(CARES_LIBRARY NAMES cares)
find_path(MHD_INCLUDE_DIR microhttpd.h)
find_library(MHD_LIBRARY NAMES microhttpd)
find_library(BOOSTTEST_LIBRARY NAMES boost_unit_test_framework)
find_package(GTest REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR})

include_directories(
    ${CMAKE_SOURCE_DIR}/thirdParty  # find thirdParty
)

include_directories(
    ${CMAKE_SOURCE_DIR}/ham  
)

include_directories(${PROJECT_SOURCE_DIR})

include_directories(${GTEST_INCLUDE_DIRS})

#link_directories(${PROJECT_BINARY_DIR}/lib)
link_directories(/usr/local/lib)

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${BUILD_TYPE})

add_subdirectory(ham/base)
add_subdirectory(ham/net)
add_subdirectory(examples)

if(BUILD_TYPE STREQUAL "RELEASE")
    add_subdirectory(benchmarks)
endif()
