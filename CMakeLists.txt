project(coro_bench)
cmake_minimum_required(VERSION 3.15)


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    link_libraries(-static-libgcc -static-libstdc++)
endif()

add_subdirectory(benchmark)

add_executable(bench
    bench.cpp
)

target_include_directories(bench PUBLIC .)
target_compile_options(bench PUBLIC)
set_property(TARGET bench PROPERTY CXX_STANDARD 20)


target_link_libraries(bench benchmark::benchmark)
