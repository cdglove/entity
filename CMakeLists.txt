cmake_minimum_required(VERSION 2.8)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)
project (entity)

###############################################################################
#
# Versioning
#
###############################################################################
set(ENTITY_VERSION_MAJOR 1)
set(ENTITY_VERSION_MINOR 0)
set(ENTITY_VERSION_PATCH 0)
set(ENTITY_VERSION_STRING ${ENTITY_VERSION_MAJOR}.${ENTITY_VERSION_MINOR}.${ENTITY_VERSION_PATCH})

###############################################################################
#
# Options
#
###############################################################################
option( ENTITY_BUILD_TESTS "Build the entity project tests." ON)
option( ENTITY_BUILD_DOCS  "Allow build the daily project docs." ON)

###############################################################################
#
# determine install path for CMake config files
#
###############################################################################
include(GNUInstallDirs)
if(WIN32 AND NOT CYGWIN)
	set(DEF_INSTALL_CMAKE_DIR CMake)
else()
	set(DEF_INSTALL_CMAKE_DIR lib/cmake/entity)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

# Make relative cmake install path absolute (needed later on)
if(NOT IS_ABSOLUTE "${INSTALL_CMAKE_DIR}")
	set(INSTALL_CMAKE_DIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_CMAKE_DIR}")
endif()

###############################################################################
#
# Configure CMake
#
###############################################################################
configure_file (
  "${PROJECT_SOURCE_DIR}/config/config.hpp.in"
  "${PROJECT_BINARY_DIR}/entity/config.hpp")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 14)

###############################################################################
#
# Configure packages
#
###############################################################################
include(contrib/cmaketools/external_git.cmake)

###############################################################################
#
# Find Boost
#
###############################################################################
set(Boost_USE_MULTI_THREADED ON)
set(Boost_USE_STATIC_LIBS ON)
find_package( Boost
  REQUIRED
  system
)

###############################################################################
#
# Find daily
#
###############################################################################
set(DAILY_BUILD_TESTS OFF CACHE BOOL "Disable libdaily tests" FORCE)
add_external_git_repo(
  "https://github.com/cdglove/daily.git"
  "master"
  "contrib/daily")
find_package(daily)

###############################################################################
#
# If using IWUY, we generate from fake cpp files to compile.  Their only`
# job is to include the ir corresponding header file.
#
###############################################################################
if(CHECK_INCLUDES_VIA_IWYU)
  file(GLOB_RECURSE PROJECT_HEADERS_RELATIVE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} entity/*.hpp)
  foreach(f IN ITEMS ${PROJECT_HEADERS_RELATIVE})
    string(REPLACE ".hpp" ".cpp" s ${f})
    file(WRITE "${PROJECT_BINARY_DIR}/gen/${s}" "#include \"${f}\"")
  endforeach(f)
  file(GLOB_RECURSE PROJECT_GENERATED_SOURCES ${PROJECT_BINARY_DIR}/gen/*.cpp)
endif()


###############################################################################
#
# Find our project files
#
###############################################################################
file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp)
file(GLOB_RECURSE PROJECT_HEADERS include/entity/*.hpp)

###############################################################################
#
# Find our project files
#
###############################################################################

set(ENTITY_SYS_INCLUDE_DIRS 
  ${Boost_INCLUDE_DIRS} 
  ${DAILY_INCLUDE_DIRS} 
)
set(ENTITY_LOCAL_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_BINARY_DIR}
)
set(ENTITY_INCLUDE_DIRS
  ${ENTITY_SYS_INCLUDE_DIRS}
  ${ENTITY_LOCAL_INDLUDE_DIRS}
)

set(ENTITY_LIBRARIES entity ${DAILY_LIBRARIES} ${Boost_LIBRARIES})

add_library(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_GENERATED_SOURCES} ${PROJECT_HEADERS})
if((CMAKE_VERSION_MAJOR VERSION_EQUAL 3) OR (CMAKE_VERSION_MAJOR VERSION_GREATER 3))
 target_include_directories(
   ${PROJECT_NAME} SYSTEM PUBLIC 
   ${ENTITY_SYS_INCLUDE_DIRS})
else()
 target_include_directories(
   ${PROJECT_NAME} PUBLIC 
   ${ENTITY_SYS_INCLUDE_DIRS})
endif()

target_include_directories(
  ${PROJECT_NAME} PUBLIC 
  ${ENTITY_LOCAL_INCLUDE_DIRS})

enable_testing()

if(ENTITY_BUILD_TESTS)
	add_subdirectory(test)
endif()

if(ENTITY_BUILD_DOCS)
	add_subdirectory(doc)
endif()

###############################################################################
#
# Enable IWYU
#
###############################################################################
if(CHECK_INCLUDES_VIA_IWYU)
  find_program(IWYU_PATH NAMES include-what-you-use iwyu)
  if(IWYU_PATH)
    set(IWYU_PATH_AND_OPTIONS ${IWYU_PATH}
      -Xiwyu --mapping_file=${IWYU_MAPFILE_DIR}gcc.stl.headers.imp
      -Xiwyu --mapping_file=${IWYU_MAPFILE_DIR}boost-all.imp
      -Xiwyu --mapping_file=${IWYU_MAPFILE_DIR}boost-all-private.imp
    )
    set_property(TARGET entity PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH_AND_OPTIONS})
  endif()
endif()

###############################################################################
#
# Export Targets
#
# (so entity can be easily used by other CMake projects)
# [see http://www.cmake.org/Wiki/CMake/Tutorials/How_to_create_a_ProjectConfig.cmake_file]
#
###############################################################################

# Add all targets to the build-tree export set
export(TARGETS entity
	FILE "${PROJECT_BINARY_DIR}/entityTargets.cmake")
# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE entity)
# Create the entityConfig.cmake and entityConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
# ... for the build tree
configure_file(entityConfig.cmake.in
	"${PROJECT_BINARY_DIR}/entityConfig.cmake" @ONLY)
# ... for the install tree
configure_file(entityConfig.cmake.in
	"${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/entityConfig.cmake" @ONLY)
# ... for both
configure_file(entityConfigVersion.cmake.in
    "${PROJECT_BINARY_DIR}/entityConfigVersion.cmake" @ONLY)
# Install the entityConfig.cmake
install(FILES
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/entityConfig.cmake"
    DESTINATION "${INSTALL_CMAKE_DIR}"
    COMPONENT dev)
# Install the export set for use with the install-tree
# install(EXPORT entityTargets
#     DESTINATION "${INSTALL_CMAKE_DIR}"
#     COMPONENT dev)



	