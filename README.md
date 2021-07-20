# Intel-TBB
Parallelization using Intel TBB <br\>

Parallel prime finder inspired by Sieve of Eratosthenes algorithm. <br\>
Implementation is designed to observe speedup using parallel_for and a body object, one of the basic concepts in Intel TBB library. <br\>
Therefore hops performed in Sieve of Eratosthenes is not performed in the Prime Finder. <br\>

The implementation is open to contribution as there are still a few optimization solutions left. <br\>
Use of bool prime array is not necessary so the storage complexity may be reduced by a basic vector containing only the primes. <br\>

Moreover, range can be adjusted manually conforming to the reasonable limits and the speedup may be further investigated to the best values. <br\>


