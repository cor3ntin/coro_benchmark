#include <benchmark/benchmark.h>
#include <generator.hpp>

template <typename Generator>
static Generator dummy() {
    co_yield 42;
}

template <typename Generator>
__attribute__((noinline))
static Generator dummy_no_inline() {
    co_yield 42;
}

template <typename Generator>
static Generator fib(int max) {
    auto a = 0, b = 1;
    for (auto n = 0; n < max; n++) {
        co_yield b;
        const auto next = a + b;
        a = b, b = next;
    }
}

template <typename Generator>
__attribute__((noinline))
static Generator fib_no_inline(int max) {
    auto a = 0, b = 1;
    for (auto n = 0; n < max; n++) {
        co_yield b;
        const auto next = a + b;
        a = b, b = next;
    }
}


template <typename Generator>
static void BM_Dummy(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    for(auto && v : dummy<Generator>()) {
        benchmark::DoNotOptimize(v);
    }
  }
}

template <typename Generator>
static void BM_DummyNoInline(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    for(auto && v : dummy_no_inline<Generator>()) {
        benchmark::DoNotOptimize(v);
    }
  }
}

template <typename Generator>
static void BM_Fib(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    for(auto && v : fib<Generator>(10000)) {
        benchmark::DoNotOptimize(v);
    }
  }
}

template <typename Generator>
static void BM_FibNoInline(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    for(auto && v : fib_no_inline<Generator>(10000)) {
        benchmark::DoNotOptimize(v);
    }
  }
}


BENCHMARK_TEMPLATE(BM_Dummy, simple::generator<uint64_t>);
BENCHMARK_TEMPLATE(BM_Dummy, recursive::generator<uint64_t>);

BENCHMARK_TEMPLATE(BM_DummyNoInline, simple::generator<uint64_t>);
BENCHMARK_TEMPLATE(BM_DummyNoInline, recursive::generator<uint64_t>);

BENCHMARK_TEMPLATE(BM_Fib, simple::generator<uint64_t>);
BENCHMARK_TEMPLATE(BM_Fib, recursive::generator<uint64_t>);

BENCHMARK_TEMPLATE(BM_FibNoInline, simple::generator<uint64_t>);
BENCHMARK_TEMPLATE(BM_FibNoInline, recursive::generator<uint64_t>);

BENCHMARK_MAIN();