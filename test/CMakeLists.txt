cmake_minimum_required(VERSION 2.8)
 
set(Boost_USE_MULTI_THREADED ON)
set(Boost_USE_STATIC_LIBS ON)
find_package( Boost
	REQUIRED
    unit_test_framework
    system
    thread
    date_time
    timer
    chrono
)

include_directories("${PROJECT_INCLUDE_DIR}")
include_directories("${PROJECT_BINARY_DIR}")

## --- Unit Testing --- ##
enable_testing()

function(create_test test_name test_source test_defines)
	add_executable(${test_name} ${test_source})
	#set_target_properties(${test_name} PROPERTIES COMPILE_DEFINITIONS ${test_defines})
	target_include_directories( ${test_name} PUBLIC ${Boost_INCLUDE_DIR} ${DAILY_INCLUDE_DIRS})
	target_link_libraries(${test_name} ${Boost_LIBRARIES})
	add_test(${test_name} ${test_name})
	set_property(TARGET ${test_name} APPEND PROPERTY COMPILE_DEFINITIONS ${test_defines})	
	if(MSVC)
		set_property(TARGET ${test_name} APPEND PROPERTY COMPILE_DEFINITIONS "_SCL_SECURE_NO_WARNINGS")
		add_definitions( "/wd4459" )
	endif()
endfunction(create_test)

#create_test(test_sparse_variadics entity_functionality_performance.cpp "TEST_SPARSE_POOLS=1;ENTITY_SUPPORT_VARIADICS=1;SIZE_OF_TEST=512")
#create_test(test_dense_variadics entity_functionality_performance.cpp "TEST_DENSE_POOLS=1;ENTITY_SUPPORT_VARIADICS=1;SIZE_OF_TEST=512")
#create_test(test_sparse_no_variadics entity_functionality_performance.cpp "TEST_SPARSE_POOLS=1;ENTITY_SUPPORT_VARIADICS=0;SIZE_OF_TEST=512")
#create_test(test_dense_no_variadics entity_functionality_performance.cpp "TEST_DENSE_POOLS=1;ENTITY_SUPPORT_VARIADICS=0;SIZE_OF_TEST=512")


#create_test(test_dense_performance_sse entity_functionality_performance.cpp "TEST_DENSE_POOLS=1;USE_SSE=1")
#create_test(test_test_raw_performance_sse_auto raw_simd_performance.cpp "SIZE_OF_TEST=(1024*512)")
#create_test(test_test_raw_performance_sse raw_simd_performance.cpp "SIZE_OF_TEST=(1024*2048);USE_EXPLICIT_LOADS=1")
#create_test(test_dense_performance_avx entity_functionality_performance.cpp "TEST_DENSE_POOLS=1;USE_AVX=1;ENTITY_SUPPORT_AVX=1")
#create_test(test_mixed_functionality entity_functionality_performance.cpp "TEST_MIXED_POOLS=1")
#create_test(TEST_non_entity_performance_0 non_entity_performance.cpp "EXTRA_PADDING=0")
#create_test(test_non_entity_performance_4 non_entity_performance.cpp "EXTRA_PADDING=4")
#create_test(test_non_entity_performance_16 non_entity_performance.cpp "EXTRA_PADDING=16")
#create_test(test_non_entity_performance_64 non_entity_performance.cpp "EXTRA_PADDING=64")
#create_test(entity_component_iterator_functionality entity_component_iterator_functionality.cpp "")
create_test(test_basic_compilation instantiate_pools.cpp "")
create_test(test_entity_lifetimes entity_lifetimes.cpp "")
create_test(test_iterator iteration.cpp "")
create_test(test_signals signals.cpp "")
create_test(test_saturated_performance entity_functionality_performance.cpp "TEST_SATURATED_POOLS=1")
create_test(test_saturated_performance_raw entity_functionality_performance.cpp "TEST_SATURATED_POOLS=1;USE_RAW_LOOPS=1")
create_test(test_sparse_performance entity_functionality_performance.cpp "TEST_SPARSE_POOLS=1;")
create_test(test_dense_performance entity_functionality_performance.cpp "TEST_DENSE_POOLS=1;")
create_test(test_sparse_performance_50 entity_functionality_performance.cpp "TEST_SPARSE_POOLS=1;TEST_DENSITY=(0.5f)")
create_test(test_dense_performance_50 entity_functionality_performance.cpp "TEST_DENSE_POOLS=1;TEST_DENSITY=(0.5f)")
create_test(test_sparse_performance_10 entity_functionality_performance.cpp "TEST_SPARSE_POOLS=1;TEST_DENSITY=(0.1f)")
create_test(test_dense_performance_10 entity_functionality_performance.cpp "TEST_DENSE_POOLS=1;TEST_DENSITY=(0.1f)")
create_test(test_sparse_performance_01 entity_functionality_performance.cpp "TEST_SPARSE_POOLS=1;TEST_DENSITY=(0.001f)")
create_test(test_dense_performance_01 entity_functionality_performance.cpp "TEST_DENSE_POOLS=1;TEST_DENSITY=(0.001f)")
create_test(test_non_entity_performance_0 non_entity_performance.cpp "EXTRA_PADDING=0")
create_test(test_manual_entity_performance manual_entity_performance.cpp "")

	