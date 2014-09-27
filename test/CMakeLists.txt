cmake_minimum_required(VERSION 2.8)
project (entity)
 
set (PROJECT_INCLUDE_DIR "../")
set (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
 
find_package( Boost )

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_INCLUDE_DIR}")

## --- Unit Testing --- ##
enable_testing()

function(create_test test_name test_source test_defines)
	add_executable(${test_name} ${test_source})
	set_target_properties(${test_name} PROPERTIES COMPILE_DEFINITIONS ${test_defines})
	target_include_directories( ${test_name} PUBLIC ${Boost_INCLUDE_DIR} ${daily_INCLUDE_DIR} )
	add_test(${test_name} ${test_name})
endfunction(create_test)

create_test(test_sparse_performance entity_functionality_performance.cpp "TEST_SPARSE_POOLS=1")
create_test(test_dense_performance entity_functionality_performance.cpp "TEST_DENSE_POOLS=1")
create_test(test_mixed_functionality entity_functionality_performance.cpp "TEST_MIXED_POOLS=1")



