cmake_minimum_required(VERSION 2.8)

option( ENTITY_ENABLE_PERFORMANCE_TESTS "Build and run performance tests (slow)" ON)
 
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

function(create_test test_name test_source test_defines)
	add_executable(${test_name} ${test_source})
	#set_target_properties(${test_name} PROPERTIES COMPILE_DEFINITIONS ${test_defines})
	target_include_directories( ${test_name} PUBLIC ${ENTITY_INCLUDE_DIRS})
	target_link_libraries(${test_name} ${Boost_LIBRARIES} ${ENTITY_LIBRARIES})
	add_test(${test_name} ${test_name})
	set_property(TARGET ${test_name} APPEND PROPERTY COMPILE_DEFINITIONS ${test_defines})	
	if(MSVC)
		set_property(TARGET ${test_name} APPEND PROPERTY COMPILE_DEFINITIONS "_SCL_SECURE_NO_WARNINGS")
		add_definitions( "/wd4459" )
	endif()
endfunction(create_test)

create_test(test.compilation instantiate_pools.cpp "")
create_test(test.entity_lifetimes entity_lifetimes.cpp "")
create_test(test.iterator iteration.cpp "")
create_test(test.signals signals.cpp "")
create_test(test.pools entity_functionality_performance.cpp "TEST_MIXED_POOLS=1;TEST_DENSITY=(0.0001f)")

if(ENTITY_ENABLE_PERFORMANCE_TESTS)
	create_test(test.perf.saturated_pool.for_each entity_functionality_performance.cpp "TEST_SATURATED_POOLS=1")
	create_test(test.perf.saturated_pool.raw_loop entity_functionality_performance.cpp "TEST_SATURATED_POOLS=1;USE_RAW_LOOPS=1")
	create_test(test.perf.sparse_pool.for_each entity_functionality_performance.cpp "TEST_SPARSE_POOLS=1;")
	create_test(test.perf.dense_pool.for_each entity_functionality_performance.cpp "TEST_DENSE_POOLS=1;")
	create_test(test.perf.sparse_pool.50 entity_functionality_performance.cpp "TEST_SPARSE_POOLS=1;TEST_DENSITY=(0.5f)")
	create_test(test.perf.dense_pool.50 entity_functionality_performance.cpp "TEST_DENSE_POOLS=1;TEST_DENSITY=(0.5f)")
	create_test(test.perf.sparse_pool.10 entity_functionality_performance.cpp "TEST_SPARSE_POOLS=1;TEST_DENSITY=(0.1f)")
	create_test(test.perf.dense_pool.10 entity_functionality_performance.cpp "TEST_DENSE_POOLS=1;TEST_DENSITY=(0.1f)")
	create_test(test.perf.sparse_pool.001 entity_functionality_performance.cpp "TEST_SPARSE_POOLS=1;TEST_DENSITY=(0.001f)")
	create_test(test.perf.dense_pool.001 entity_functionality_performance.cpp "TEST_DENSE_POOLS=1;TEST_DENSITY=(0.001f)")
	create_test(test.perf.manual_struct non_entity_performance.cpp "EXTRA_PADDING=0")
	create_test(test.perf.manual_pools manual_entity_performance.cpp "")
endif()

	