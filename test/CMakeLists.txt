set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(locked-test
    test.cc
)

target_compile_features(locked-test
    PRIVATE
        cxx_std_17
)

target_link_libraries(locked-test
    PRIVATE
        locked
)

option(LOCKED_WARNINGS_AS_ERRORS "Treat warnings as errors" TRUE)

if (MSVC)
	target_compile_options(locked-test
		PRIVATE
			/W4
	)

	if (LOCKED_WARNINGS_AS_ERRORS)
		target_compile_options(locked-test
			PRIVATE
				/WX
		)
	endif()

	if (MSVC_VERSION GREATER_EQUAL 1910) # Visual Studio 2017 or greater
		target_compile_options(locked-test
			PRIVATE
				/permissive-
				/diagnostics:caret
		)
	endif()
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if (LOCKED_WARNINGS_AS_ERRORS)
		target_compile_options(locked-test
			PRIVATE
				-Werror
		)
	endif()

	target_compile_options(locked-test
		PRIVATE
			-Wall
			-Wextra
			-pedantic
			-Wdouble-promotion
			-Wconversion
			-Wnull-dereference
			-Wsign-conversion
	)
endif()
