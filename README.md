# Intel-TBB
Parallelization using Intel TBB  <br />

Parallel prime finder inspired by Sieve of Eratosthenes algorithm.  <br />
Implementation is designed to observe speedup using parallel_for and a body object, one of the basic concepts in Intel TBB library.  <br />
Therefore hops performed in Sieve of Eratosthenes is not performed in this Prime Finder being different from the Sieve of Eratosthenes, instead a sequential pass is employed for each k value.  <br />

The implementation is open to contribution as there are still a few optimizations left. <br />
Prime array may not be necessarily used so that the storage complexity may also be reduced by using a vector containing only the primes. This vector may be accumulated to the result using split/join functionality of parallel_reduce.  <br />

Alternatively, instead of automatic chunk splitting, grainsize can be adjusted manually conforming to the reasonable limits or affinity partitioners can be used so the speedup may be further investigated to the best values.  <br />

Hardware Details: <br />
Environment: Big Sur, MacBook Pro 2019  <br />
Processor: 1.4 GHz Quad-Core Intel i5  <br />
Memory: 8 GB 2133 MHz LPDDR3 <br />
<br />
Parallel overhead is compensated when input size is nearly a million. <br />

Compilation: <br />
g++ -g -std=c++17 eratosthenes_tbb.cpp -pthread -ltbb <br />

Run:  <br />
./exec 10000000   <br />






