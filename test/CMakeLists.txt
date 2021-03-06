cmake_minimum_required(VERSION 3.2)

option( ENTITY_ENABLE_PERFORMANCE_TESTS "Build and run performance tests (slow)" ON)

###############################################################################
#
# Find daily -- needed for performance counters.
#
###############################################################################
set(DAILY_BUILD_TESTS OFF CACHE BOOL "Disable libdaily tests" FORCE)
add_external_git_repo(
  URL "https://github.com/cdglove/daily.git"
  TAG "master"
  PREFIX "contrib/daily"
  ALWAYS_UPDATE)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
add_external_git_repo(
	URL "https://github.com/google/benchmark.git"
	TAG "master"
	PREFIX "contrib/google/benchmark"
	ALWAYS_UPDATE)
 
set(Boost_USE_MULTI_THREADED ON)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost
	REQUIRED
    unit_test_framework
    system
    thread
    date_time
    chrono
)

function(create_test test_name test_source test_defines)
	add_executable(${test_name} ${test_source})
	target_link_libraries(${test_name} PUBLIC ${Boost_LIBRARIES} daily entity benchmark)
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

if(ENTITY_ENABLE_PERFORMANCE_TESTS)

	# Re-find boost libraries to override ${Boost_LIBRARIES}
	find_package( Boost REQUIRED )
	# create_test(test.perf.saturated.raw entity_performance.cpp "TEST_SATURATED_POOLS=1;USE_RAW_LOOPS=1")
	# create_test(test.perf.saturated.fetch entity_performance.cpp "TEST_SATURATED_POOLS=1;USE_INDEXED_LOOPS=1")
	# create_test(test.perf.saturated.helper entity_performance.cpp "TEST_SATURATED_POOLS=1;USE_GET_HELPER=1")
	# create_test(test.perf.saturated.zip entity_performance.cpp "TEST_SATURATED_POOLS=1;USE_ZIP_ITERATOR=1")
	# create_test(test.perf.saturated.range entity_performance.cpp "TEST_SATURATED_POOLS=1;USE_RANGE=1")
	# create_test(test.perf.saturated.optional entity_performance.cpp "TEST_SATURATED_POOLS=1;USE_OPTIONAL_ITERATORS=1")
	# create_test(test.perf.saturated.indexed entity_performance.cpp "TEST_SATURATED_POOLS=1;USE_INDEXED_OPTIONAL_ITERATORS=1")
	# create_test(test.perf.dense.fetch entity_performance.cpp "TEST_DENSE_POOLS=1;USE_INDEXED_LOOPS=1")
	# create_test(test.perf.dense.helper entity_performance.cpp "TEST_DENSE_POOLS=1;USE_GET_HELPER=1")
	# create_test(test.perf.dense.zip entity_performance.cpp "TEST_DENSE_POOLS=1;USE_ZIP_ITERATOR=1")
	# create_test(test.perf.dense.range entity_performance.cpp "TEST_DENSE_POOLS=1;USE_RANGE=1")
	# create_test(test.perf.dense.optional entity_performance.cpp "TEST_DENSE_POOLS=1;USE_OPTIONAL_ITERATORS=1")
	# create_test(test.perf.dense.indexed entity_performance.cpp "TEST_DENSE_POOLS=1;USE_INDEXED_OPTIONAL_ITERATORS=1")
	# create_test(test.perf.sparse.fetch entity_performance.cpp "TEST_DENSE_POOLS=1;USE_INDEXED_LOOPS=1")
	# create_test(test.perf.sparse.helper entity_performance.cpp "TEST_SPARSE_POOLS=1;USE_GET_HELPER=1")
	# create_test(test.perf.sparse.zip entity_performance.cpp "TEST_SPARSE_POOLS=1;USE_ZIP_ITERATOR=1")
	# create_test(test.perf.sparse.range entity_performance.cpp "TEST_SPARSE_POOLS=1;USE_RANGE=1")
	# create_test(test.perf.sparse.optional entity_performance.cpp "TEST_SPARSE_POOLS=1;USE_OPTIONAL_ITERATORS=1")
	# create_test(test.perf.sparse.indexed entity_performance.cpp "TEST_SPARSE_POOLS=1;USE_INDEXED_OPTIONAL_ITERATORS=1")
	# create_test(test.perf.sparse_pool.for_each entity_performance.cpp "TEST_SPARSE_POOLS=1;")
	# create_test(test.perf.dense_pool.for_each entity_performance.cpp "TEST_DENSE_POOLS=1;")
	# create_test(test.perf.sparse_pool.90 entity_performance.cpp "TEST_SPARSE_POOLS=1;USE_RANGE=1;TEST_DENSITY=(0.9f)")
	# create_test(test.perf.dense_pool.90 entity_performance.cpp "TEST_DENSE_POOLS=1;USE_RANGE=1;TEST_DENSITY=(0.9f)")
	# create_test(test.perf.sparse_pool.50 entity_performance.cpp "TEST_SPARSE_POOLS=1;USE_RANGE=1;TEST_DENSITY=(0.5f)")
	# create_test(test.perf.dense_pool.50 entity_performance.cpp "TEST_DENSE_POOLS=1;USE_RANGE=1;TEST_DENSITY=(0.5f)")
	# create_test(test.perf.sparse_pool.10 entity_performance.cpp "TEST_SPARSE_POOLS=1;USE_RANGE=1;TEST_DENSITY=(0.1f)")
	# create_test(test.perf.dense_pool.10 entity_performance.cpp "TEST_DENSE_POOLS=1;USE_RANGE=1;TEST_DENSITY=(0.1f)")
	# create_test(test.perf.sparse_pool.001 entity_performance.cpp "TEST_SPARSE_POOLS=1;USE_RANGE=1;TEST_DENSITY=(0.001f)")
	# create_test(test.perf.dense_pool.001 entity_performance.cpp "TEST_DENSE_POOLS=1;USE_RANGE=1;TEST_DENSITY=(0.001f)")
	
	#create_test(benchmark.manual_struct non_entity_performance.cpp "EXTRA_PADDING=0")
	#create_test(benchmark.manual_pools manual_entity_performance.cpp "")
	set(BENCHMARK_FILES benchmark.pool_iteration.cpp benchmark.base_iteration.cpp benchmark.main.cpp)
	add_executable(benchmark.iteration ${BENCHMARK_FILES})
	target_link_libraries(benchmark.iteration PUBLIC ${Boost_LIBRARIES} entity benchmark)
	add_test(benchmark.iteration benchmark.iteration)

	if(MSVC)
		set_property(TARGET benchmark.iteration APPEND PROPERTY COMPILE_DEFINITIONS "_SCL_SECURE_NO_WARNINGS")
		add_definitions( "/wd4459" )
	endif()
endif()

	