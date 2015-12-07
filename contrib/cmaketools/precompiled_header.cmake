function(target_precompiled_header target header)
	get_filename_component(header_full_path ${header} ABSOLUTE)
	set_source_files_properties(${header} PROPERTIES LANGUAGE CXX)
	add_library(${target}.pch OBJECT ${header_full_path})

	if(NOT DEFINED CMAKE_PCH_COMPILER_TARGETS)
		# this will be executed in just before makefile generation
		variable_watch(CMAKE_BACKWARDS_COMPATIBILITY __watch_pch_last_hook)
	endif()

	# keep track of pch targets
	list(APPEND CMAKE_PCH_COMPILER_TARGETS ${target})
	set(CMAKE_PCH_COMPILER_TARGETS "${CMAKE_PCH_COMPILER_TARGETS}" PARENT_SCOPE)
	#list(APPEND CMAKE_PCH_COMPILER_TARGET_FLAGS ${new_target_flags})
	#set(CMAKE_PCH_COMPILER_TARGET_FLAGS "${CMAKE_PCH_COMPILER_TARGET_FLAGS}" PARENT_SCOPE)

	# target_compile_options(${target} PUBLIC "-include ${header_full_path}")
	# target_include_directories(
	# 	${target}
	# 	BEFORE PUBLIC
	# 	${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${target}.pch.dir)
endfunction()

# copies all compile definitions, flags and options to .pch subtarget
function(__watch_pch_last_hook variable access value)
	list(LENGTH CMAKE_PCH_COMPILER_TARGETS length)
	message("*********** Targets: ${CMAKE_PCH_COMPILER_TARGETS}")
	foreach(index RANGE 0 ${length})
		list(GET CMAKE_PCH_COMPILER_TARGETS ${index} target)
		message("*********** Patching: ${target}")
		#list(GET CMAKE_PCH_COMPILER_TARGET_FLAGS ${index} flags)
		foreach(
			property
			COMPILE_DEFINITIONS
			COMPILE_DEFINITIONS_DEBUG
			COMPILE_DEFINITIONS_MINSIZEREL
			COMPILE_DEFINITIONS_RELEASE
			COMPILE_DEFINITIONS_RELWITHDEBINFO
			COMPILE_FLAGS
			COMPILE_OPTIONS
			INCLUDE_DIRECTORIES
			CXX_STANDARD)

			get_target_property(value ${target} ${property})
			message("*********** ${value}")
			if(NOT "${value}" STREQUAL "value-NOTFOUND")
				set_target_properties(${target}.pch PROPERTIES ${property} "${value}")
			endif()
		endforeach()
	endforeach()
endfunction()