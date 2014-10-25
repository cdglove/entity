###############################################################################
#
# Configure package management
#
###############################################################################
find_package(Git)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "Package management requires Git.")
endif()

include (CMakeParseArguments)

###############################################################################
#
# clone_external_git_repo
# Clones a repo into a directory at configure time.  For more automated
# project configuration, consider add_external_git_repo
#
###############################################################################
function(clone_external_git_repo URL TAG TARGET_DIR)
  #set(single_value_args URL TAG TARGET_DIR) 
  #cmake_parse_arguments(t "${single_value_args}" ${ARGN})

  if(NOT EXISTS "${TARGET_DIR}/.git")
    message(STATUS "Cloning repo ${URL}")
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" clone ${URL} --branch ${TAG} ${TARGET_DIR}
      RESULT_VARIABLE error_code)
    if(error_code)
      message(FATAL_ERROR "Failed to clone ${URL}")
    endif()
  else()
    message(STATUS "Updating repo ${URL}")
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" pull
      WORKING_DIRECTORY ${TARGET_DIR})
    if(error_code)
      message(STATUS "Failed to update ${URL}")
    endif()
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" checkout ${TAG} 
      WORKING_DIRECTORY ${TARGET_DIR})
    if(error_code)
      message(STATUS "Failed to chechout ${TAG} on ${URL}")
    endif()
  endif()
endfunction(clone_external_git_repo)

###############################################################################
#
# add_external_git_repo
# Adds a prepository into a prefix and automatically configurs it for use
# using defalt locations. For more control, use clone_external_git_repo
#
###############################################################################
function(add_external_git_repo URL TAG PREFIX)
  clone_external_git_repo(${URL} ${TAG} "${PROJECT_SOURCE_DIR}/${PREFIX}")
  add_subdirectory("${PROJECT_SOURCE_DIR}/${PREFIX}")
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${PROJECT_BINARY_DIR}/${PREFIX}" )
endfunction(add_external_git_repo)