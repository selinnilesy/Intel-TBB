# Intel-TBB
Parallelization using Intel TBB  <br />

Parallel prime finder inspired by Sieve of Eratosthenes algorithm.  <br />
Implementation is designed to observe speedup using parallel_for&parallel_reduce within a body object, one of the basic concepts in Intel TBB library.  <br />
- eratosthenes_tbb_reduce.cpp contains the implementation with parallel_reduce and it determines the chunking manually, operating on a quad-core processor. Speedup for 10^6 input size and 16 threads : 150  <br />

<br />

- eratosthenes_tbb.cpp contains the implementation with parallel_for and it determines the chunking automatically, having a poor speedup. Alternatively, instead of automatic chunk splitting, grainsize can be adjusted manually conforming to the reasonable limits or affinity partitioners can be used so the speedup may be further investigated to the best values. 250000 has given the best when size=10^6 within simple partitioner. <br />

Hardware Details: <br />
Environment: Big Sur, MacBook Pro 2019  <br />
Processor: 1.4 GHz Quad-Core Intel i5  <br />
Memory: 8 GB 2133 MHz LPDDR3 <br />
<br />
Parallel overhead is compensated when input size is nearly a million. <br />

Compilation: <br />
g++ -g -std=c++17 eratosthenes_tbb.cpp -pthread -ltbb <br />

Run:  <br />
./exec 10000000 <thread_count> [silent]  <br />






