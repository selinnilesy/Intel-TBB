## Intel-TBB
Parallelization using Intel TBB  <br />

Parallel prime finder inspired by Sieve of Eratosthenes algorithm.  <br />
Implementation is designed to observe speedup using parallel_for&parallel_reduce within a body object, one of the basic concepts in Intel TBB library.  <br />
- eratosthenes_tbb_reduce.cpp contains the implementation with parallel_reduce to be able to describe a user-defined iteration space called SieveRange (syntactic obligation for this solution), needed by the nature of the algorithm. This implementation uses a stride over the iteration space instead of computing modulo results for each number in the user input data.  However, the reduction operation which is not needed in the problem definition may cause overhead. <br />
The for loop implemented in the main is described for finding the next stride (k) and inherently sequential, however, another parallel_reduce class called FindNextK was also implemented to compare results with its parallelized version. <br />
Speedup: between 2-3 <br />

- eratosthenes_tbb.cpp contains the implementation with parallel_for and it determines the chunking automatically and a biy far away from the original description of the algorithm, for parallelization reasons. This implementation scans the data array one by one and computes modulo operation on each number. Stride concept of the Sieve of Eratosthenes cannot be employed since iteration ranges are determined on the flight. However, the striking of composite numbers and prime detection mechanism defined in the algorithm is still used. <br />
Grainsize also can be adjusted manually conforming to the reasonable limits or affinity partitioners can be used so the speedup may be further investigated to the best values. G=250000 has given the best when using simple partitioner for the input size of million and quadcores. <br />
Speedup: 4 (Ideal Speedup) <br />

Parallel overhead is mostly compansated after input size 10 million.  <br />

# SuperComputer: Hawk <br />
Nodes: PBS <br />
Node Type: rome <br />
CPU: AMD7742 <br />
\# of Cores: User defined in provided script <br />
Compilation: Please use the provided Makefile and submit jobs with given scripts.


# MacOS
Compilation: <br />
g++ -g -std=c++17 eratosthenes_tbb.cpp -pthread -ltbb <br />

Run:  <br />
./exec 10000000 <thread_count> [silent]  <br />

Hardware Details: <br />
Environment: Big Sur, MacBook Pro 2019  <br />
Processor: 1.4 GHz Quad-Core Intel i5  <br />
Memory: 8 GB 2133 MHz LPDDR3 <br />







