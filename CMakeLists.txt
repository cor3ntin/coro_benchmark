
link_libraries(-static-libgcc -static-libstdc++)

#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

add_subdirectory(benchmark)

add_executable(bench
    bench.cpp
)

target_include_directories(bench PUBLIC .)
target_compile_options(bench PUBLIC)
set_property(TARGET bench PROPERTY CXX_STANDARD 20)


target_link_libraries(bench benchmark::benchmark)
