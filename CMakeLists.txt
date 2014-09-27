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
	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has may not support Please use a different C++ compiler if building fails.")
endif()

if(DEFINED ENV{_DAILY_ROOT})
	set( daily_INCLUDE_DIR $ENV{DAILY_ROOT} )
else()
	include(ExternalProject)
	ExternalProject_Add(
		daily
		GIT_REPOSITORY "https://github.com/cdglove/daily.git"
		GIT_TAG "master"
		UPDATE_COMMAND ""
		INSTALL_COMMAND "" 
	)
	ExternalProject_Get_Property(daily install_dir)
	set( daily_INCLUDE_DIR ${install_dir}/src/daily )
endif()

add_subdirectory(test)





	