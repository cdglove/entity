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
function(add_external_git_repo URL TAG TARGET_DIR)
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
endfunction(add_external_git_repo)