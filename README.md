# Intel-TBB
Parallelization using Intel TBB  <br />

Parallel prime finder inspired by Sieve of Eratosthenes algorithm.  <br />
Implementation is designed to observe speedup using parallel_for&parallel_reduce within a body object, one of the basic concepts in Intel TBB library.  <br />
- eratosthenes_tbb_reduce.cpp contains the implementation with parallel_reduce and it describes a user-defined iteration space called SieveRange for chunking, operating on a quad-core processor. This implementation has to use a user-defined space since there is a stride employed instead of computing mod values one by one on the numbers.  <br />
For loop needed for finding the next stride (k) is inherently sequential, however, another parallel_reduce class called FindNextK was also implemented to compare results. <br />
Parallel overhead is compensated after input size 10 million. Speedup: nearly 2. <br />

- eratosthenes_tbb.cpp contains the implementation with parallel_for and it determines the chunking automatically. Alternatively, instead of automatic chunk splitting, grainsize can be adjusted manually conforming to the reasonable limits or affinity partitioners can be used so the speedup may be further investigated to the best values. G=250000 has given the best when using simple partitioner. <br />
Speedup: 4 (Ideal Speedup) starting from an input size of million. <br />


Compilation: <br />
g++ -g -std=c++17 eratosthenes_tbb.cpp -pthread -ltbb <br />

Run:  <br />
./exec 10000000 <thread_count> [silent]  <br />

Hardware Details: <br />
Environment: Big Sur, MacBook Pro 2019  <br />
Processor: 1.4 GHz Quad-Core Intel i5  <br />
Memory: 8 GB 2133 MHz LPDDR3 <br />







