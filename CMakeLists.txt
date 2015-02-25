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
include(contrib/cmaketools/enable_cpp11.cmake)

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
# Add entity library
#
###############################################################################
file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp)
file(GLOB_RECURSE PROJECT_HEADERS entity/*.hpp)

set(ENTITY_INCLUDE_DIRS 
	${Boost_INCLUDE_DIR} 
	${DAILY_INCLUDE_DIRS} 
	${PROJECT_SOURCE_DIR}
	${PROJECT_BINARY_DIR}
)
set(ENTITY_LIBRARIES entity ${DAILY_LIBRARIES} ${Boost_LIBRARIES})

add_library( ${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS} )
target_include_directories( ${PROJECT_NAME} PUBLIC ${ENTITY_INCLUDE_DIRS})

enable_testing()

if(ENTITY_BUILD_TESTS)
	add_subdirectory(test)
endif()

if(ENTITY_BUILD_DOCS)
	add_subdirectory(doc)
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



	