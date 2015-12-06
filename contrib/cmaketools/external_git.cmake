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
function(clone_external_git_repo)
  set(oneValueArgs URL TARGET_DIR TAG ALWAYS_UPDATE)
  cmake_parse_arguments(clone_external "" "${oneValueArgs}" "" ${ARGN} )

  if(NOT EXISTS "${clone_external_TARGET_DIR}/.git")
    message(STATUS "Cloning repo ${clone_external_URL}")
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" clone ${clone_external_URL} --branch ${clone_external_TAG} ${clone_external_TARGET_DIR}
      RESULT_VARIABLE error_code)
    if(error_code)
      message(FATAL_ERROR "Failed to clone ${clone_external_URL}")
    endif()
  elseif(${clone_external_ALWAYS_UPDATE})
    message(STATUS "Updating repo ${clone_external_URL}")
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" pull
      WORKING_DIRECTORY ${clone_external_TARGET_DIR})
    if(error_code)
      message(STATUS "Failed to update ${clone_external_URL}")
    endif()
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" checkout ${clone_external_TAG} 
      WORKING_DIRECTORY ${clone_external_TARGET_DIR})
    if(error_code)
      message(STATUS "Failed to chechout ${clone_external_TAG} on ${clone_external_URL}")
    endif()
  endif()
endfunction(clone_external_git_repo)

###############################################################################
#
# add_external_git_repo
# Adds a prepository into a prefix and automatically configures it for use
# using defalt locations. For more control, use clone_external_git_repo
#
###############################################################################
function(add_external_git_repo)
  set(options ALWAYS_UPDATE ADD_SUBDIR)
  set(oneValueArgs URL PREFIX TAG)
  cmake_parse_arguments(external_git "${options}" "${oneValueArgs}" "" ${ARGN} )

  clone_external_git_repo(
    URL ${external_git_URL} 
    TAG ${external_git_TAG} 
    TARGET_DIR "${PROJECT_SOURCE_DIR}/${external_git_PREFIX}"
    ALWAYS_UPDATE ${external_git_ALWAYS_UPDATE})

  get_filename_component(full_path_source_dir "${PROJECT_SOURCE_DIR}/${external_git_PREFIX}" ABSOLUTE)
  get_filename_component(full_path_bin_dir "${PROJECT_BINARY_DIR}/${external_git_PREFIX}" ABSOLUTE)
  if(${external_git_ADD_SUBDIR})
    add_subdirectory(${full_path_source_dir} ${full_path_bin_dir})
  endif()
endfunction(add_external_git_repo)