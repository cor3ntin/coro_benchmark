-----------------------------------------------------------------------------------
Benchmark                                         Time             CPU   Iterations
-----------------------------------------------------------------------------------
BM_Dummy<simple::generator<uint64_t>>          12.5 ns         12.5 ns     56173021
BM_Dummy<recursive::generator<uint64_t>>       13.3 ns         13.3 ns     52546954
BM_Fib<simple::generator<uint64_t>>           29462 ns        29458 ns        23735
BM_Fib<recursive::generator<uint64_t>>        34650 ns        34646 ns        20170
