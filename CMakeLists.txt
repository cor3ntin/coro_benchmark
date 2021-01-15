link_libraries(-static-libgcc -static-libstdc++)

add_subdirectory(benchmark)

add_executable(bench
    bench.cpp
)

target_include_directories(bench PUBLIC .)
set_property(TARGET bench PROPERTY CXX_STANDARD 20)


target_link_libraries(bench benchmark::benchmark)
