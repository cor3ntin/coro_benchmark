-------------------------------------------------------------------------------------------
GCC Benchmark                                              Time             CPU   Iterations
-------------------------------------------------------------------------------------------
BM_Dummy<simple::generator<uint64_t>>                  12.4 ns         12.4 ns     56679032
BM_Dummy<recursive::generator<uint64_t>>               13.4 ns         13.4 ns     52506378
BM_DummyNoInline<simple::generator<uint64_t>>          14.1 ns         14.1 ns     49924744
BM_DummyNoInline<recursive::generator<uint64_t>>       15.2 ns         15.2 ns     46475027
BM_Fib<simple::generator<uint64_t>>                   30283 ns        30279 ns        23314
BM_Fib<recursive::generator<uint64_t>>                29110 ns        29106 ns        24033
BM_FibNoInline<simple::generator<uint64_t>>           30099 ns        30095 ns        23279
BM_FibNoInline<recursive::generator<uint64_t>>        29004 ns        29000 ns        24152
BM_Range                                               2457 ns         2456 ns       286840
BM_RangeSymmetricTransfer                              2274 ns         2274 ns       309655
BM_DeepRecursion                                   58801348 ns     58791879 ns           12
BM_DeepSymmetricTransfer                             338736 ns       338661 ns         2108


-------------------------------------------------------------------------------------------
Clang Benchmark                                           Time             CPU   Iterations
-------------------------------------------------------------------------------------------
BM_Dummy<simple::generator<uint64_t>>                 0.235 ns        0.235 ns   1000000000
BM_Dummy<recursive::generator<uint64_t>>               2.36 ns         2.36 ns    297561323
BM_DummyNoInline<simple::generator<uint64_t>>          13.5 ns         13.5 ns     51090377
BM_DummyNoInline<recursive::generator<uint64_t>>       13.7 ns         13.7 ns     51263462
BM_Fib<simple::generator<uint64_t>>                    4160 ns         4159 ns       168659
BM_Fib<recursive::generator<uint64_t>>                24568 ns        24564 ns        28499
BM_FibNoInline<simple::generator<uint64_t>>           23653 ns        23649 ns        29912
BM_FibNoInline<recursive::generator<uint64_t>>        24552 ns        24547 ns        28663
BM_Range                                               2321 ns         2320 ns       298459
BM_RangeSymmetricTransfer                              2134 ns         2134 ns       327160
BM_DeepRecursion                                   43670266 ns     43660966 ns           17
BM_DeepSymmetricTransfer                             427955 ns       427876 ns         1707


-------------------------------------------------------------------------------------------
MSVC Benchmark                                           Time             CPU   Iterations
-------------------------------------------------------------------------------------------
BM_Dummy<simple::generator<uint64_t>>                  61.9 ns         62.8 ns     11200000
BM_Dummy<recursive::generator<uint64_t>>               63.7 ns         64.2 ns     11200000
BM_DummyNoInline<simple::generator<uint64_t>>          63.8 ns         64.2 ns     11200000
BM_DummyNoInline<recursive::generator<uint64_t>>       64.4 ns         64.2 ns     11200000
BM_Fib<simple::generator<uint64_t>>                   54114 ns        54688 ns        10000
BM_Fib<recursive::generator<uint64_t>>                71496 ns        71498 ns         8960
BM_FibNoInline<simple::generator<uint64_t>>           51714 ns        51563 ns        10000
BM_FibNoInline<recursive::generator<uint64_t>>        71052 ns        71150 ns        11200
BM_Range                                               4387 ns         4395 ns       160000
BM_RangeSymmetricTransfer                              5279 ns         5313 ns       100000
BM_DeepRecursion                                  224052033 ns    223958333 ns            3
BM_DeepSymmetricTransfer                            4760914 ns      4849138 ns          145