cmake_minimum_required(VERSION 3.10)

project(locked LANGUAGES CXX VERSION 0.1)

add_library(locked INTERFACE)
target_include_directories(locked
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
set(locked_sources
    ${CMAKE_CURRENT_LIST_DIR}/include/locked/locked.hh
)
target_sources(locked INTERFACE $<BUILD_INTERFACE:${locked_sources}>)
target_compile_features(locked
    INTERFACE
        cxx_std_17
)

option(LOCKED_TESTING "Enable unit tests" FALSE)

if (LOCKED_TESTING)
    add_subdirectory(test)
endif()

