# Intel-TBB
Parallelization using Intel TBB  <br />

Parallel prime finder inspired by Sieve of Eratosthenes algorithm.  <br />
Implementation is designed to observe speedup using parallel_for&parallel_reduce within a body object, one of the basic concepts in Intel TBB library.  <br />
- eratosthenes_tbb_reduce.cpp contains the implementation with parallel_reduce and it describes a user-defined iteration space called SieveRange for chunking, operating on a quad-core processor. Simple partitioners and other partitioner alternatives are observed previously. For loop needed for next stride finder is inherently sequential, however, another parallel_reduce class called FindNextK was also implemented to compare results. Parallel overhead is compensated after input size 10 million. Speedup is nearly 2. <br />

- eratosthenes_tbb.cpp contains the implementation with parallel_for and it determines the chunking automatically. Alternatively, instead of automatic chunk splitting, grainsize can be adjusted manually conforming to the reasonable limits or affinity partitioners can be used so the speedup may be further investigated to the best values. G=250000 has given the best when size=10^6 within simple partitioner. Speedup: Very close to ideal speedup 4 starting from an input size of million. <br />


Compilation: <br />
g++ -g -std=c++17 eratosthenes_tbb.cpp -pthread -ltbb <br />

Run:  <br />
./exec 10000000 <thread_count> [silent]  <br />

Hardware Details: <br />
Environment: Big Sur, MacBook Pro 2019  <br />
Processor: 1.4 GHz Quad-Core Intel i5  <br />
Memory: 8 GB 2133 MHz LPDDR3 <br />







