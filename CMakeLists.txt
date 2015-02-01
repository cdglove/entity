cmake_minimum_required(VERSION 2.8)
project (entity)

set(ENTITY_VERSION_MAJOR 1)
set(ENTITY_VERSION_MINOR 0)
set(ENTITY_VERSION_PATCH 0)
set(ENTITY_VERSION_STRING ${ENTITY_VERSION_MAJOR}.${ENTITY_VERSION_MINOR}.${ENTITY_VERSION_PATCH})

option( ENTITY_BUILD_TESTS "Build the daily project tests." ON)

configure_file (
  "${PROJECT_SOURCE_DIR}/config/config.hpp.in"
  "${PROJECT_BINARY_DIR}/entity/config.hpp")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR})

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
	if(NOT MSVC)
		message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has may not support Please use a different C++ compiler if building fails.")
	endif()
endif()

###############################################################################
#
# Configure packages
#
###############################################################################
include(contrib/cmaketools/external_git.cmake)
set(DAILY_BUILD_TESTS OFF CACHE BOOL "Disable libdaily tests" FORCE)

###############################################################################
#
# Find daily
#
###############################################################################
add_external_git_repo(
  "https://github.com/cdglove/daily.git"
  "master"
  "contrib/daily")
find_package(daily)

###############################################################################
#
# Add entity library
#
###############################################################################
file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp)
file(GLOB_RECURSE PROJECT_HEADERS entity/*.hpp)
add_library( ${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS} )
target_include_directories( ${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIR} ${DAILY_INCLUDE_DIRS})

enable_testing()

if(ENTITY_BUILD_TESTS)
	add_subdirectory(test)
endif()





	