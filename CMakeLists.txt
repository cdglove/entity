cmake_minimum_required(VERSION 2.8)
project (entity)
 
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PROJECT_INCLUDE_DIR ".")

enable_testing()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	if(MSVC EQUAL FALSE)
		message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has may not support Please use a different C++ compiler if building fails.")
	endif()
endif()

if(MSVC EQUAL FALSE)
	
endif()

if(DEFINED ENV{_DAILY_ROOT})
	set( daily_INCLUDE_DIR $ENV{DAILY_ROOT} )
else()
	include(ExternalProject)
	ExternalProject_Add(
		daily
		GIT_REPOSITORY "https://github.com/cdglove/daily.git"
		GIT_TAG "master"
		SOURCE_DIR "${PROJECT_SOURCE_DIR}/contrib/daily"
		UPDATE_COMMAND ""
		INSTALL_COMMAND "" 
	)
	set( daily_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/contrib/daily" )
endif()

file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp)
file(GLOB_RECURSE PROJECT_HEADERS entity/*.h)
add_library( ${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS} ) 

add_subdirectory(test)





	