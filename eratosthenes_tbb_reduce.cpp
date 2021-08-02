//
// Created by Selin Yıldırım on 20.07.2021.
//
#include "oneapi/tbb.h"
#include <iostream>
#include <stdlib.h>     /* atoi */
#include <cstring>
#include <numeric>
#include <chrono>

using namespace oneapi::tbb;
using namespace std;

/* This class splits the iteration space according to the current stride and grainsize.
 * Range  is selected according to the first number the stride can divide without a remainder.
 * */
class SieveRange {
    int my_stride, my_grainsize;
    int my_begin,my_end;
    bool assert_okay() const {
        assert(my_begin % my_stride == 0);
        assert(my_begin <= my_end);
        assert(my_stride <= my_grainsize);
        return true;
    }
public:
    bool is_divisible() const { return my_end - my_begin > my_grainsize; }
    bool empty() const { return my_end <= my_begin
    ; }
    // Split r into subranges r and *this
    SieveRange( SieveRange& r, split ): my_stride(r.my_stride),
                                        my_grainsize(r.my_grainsize),
                                        my_end(r.my_end) {
        assert(r.is_divisible());
        assert(r.assert_okay());
        int middle = r.my_begin + (r.my_end - r.my_begin + r.my_stride - 1) / 2;
        middle = middle / my_stride * my_stride;
        my_begin = middle;
        r.my_end = middle;
        assert(assert_okay());
        assert(r.assert_okay());
    }
    int begin() const {
        return my_begin;
    }
    int end() const {
        return my_end;
    }
    SieveRange(int begin, int end, int stride, int grainsize)
            : my_begin(begin),
              my_end(end),
              my_stride(stride),
              my_grainsize(grainsize < stride ? stride : grainsize) {
        assert(assert_okay());
    }
};
/* This is the actual class containing the functor of Sieve of Eratosthenes.
 * Stride k is used to strike the composite numbers of each range.
 * */
class ApplySieve {
    bool* isPrime;
    const int k;
public:
    void operator()( const SieveRange& range ) const {
        bool *copyPrime = isPrime;
        int local_k = k;
        size_t i = range.begin();
        size_t end = range.end();
        for( ; i<end;  i+=local_k ){
            copyPrime[i] = 0;
        }
    }
    ApplySieve(int k_init, bool* isPrimeInit) :  k(k_init), isPrime(isPrimeInit) {}

    ApplySieve( ApplySieve& x, split ) :  k(x.k), isPrime(x.isPrime) {}
    void join( const ApplySieve& y ) {}
};
/* This class is written for comparison purposes.
 * The for loop used to select the next stride (k) is tried to be parallelized here.
 * Since the first unmarked no. with the min index needs to be selected,
 * the loop is inherently sequential. Extra if checks of the joins are some of the bottleneck.
 *
class FindNextK {
     const bool* primes;
     const size_t n;
public:
    int k;
    void operator()( const blocked_range<size_t>& range ) {
        static const bool *copyPrime = primes;
        int i = range.begin();
        int end = range.end();
        for( ; i<end;  i++ ){
            if(copyPrime[i] && i<k) {
                k = i;
                break;
            }
        }
    }
    FindNextK(int k_init, bool* isPrimeInit, size_t size_init) :  k(k_init), primes(isPrimeInit), n(size_init) {}

    FindNextK( FindNextK& x, split ) : k(INT_MAX), primes(x.primes), n(x.n) {}
    void join( const FindNextK& y ) {
        if(y.k < this->k) {
            k=y.k;
        }
    }

};
*/

inline void ParallelApplySieve( int* a, size_t n, bool* primes , int local_k) {

    ApplySieve obj(local_k, primes);
    // set the range to be sieved as k^2 to n as the algorithm suggests.
    // there are 4 local cores, so the grainsize is handled in hardcoded way.
    parallel_reduce(SieveRange(local_k*local_k, n, local_k, n/4), // begin end stride grain size
                                 obj,
                                 oneapi::tbb::simple_partitioner());
}
std::chrono::duration<double> SerialSieve( size_t n, bool silent) {
    const auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> seqTime;
    int i, j, local_k = 2;
    bool *primes = new bool[n];
    memset(primes, 1, n);

    primes[0] = primes[1] = 0;
    while(1) {
        // mark composites in the algorithm's range, k^2 to n.
        for (j=local_k*local_k; j <= n; j+=local_k){
            primes[j] = 0;
        }
        // update k to next prime number
        for (i = local_k + 1; i < n; i++) {
            if (primes[i]) {
                local_k = i;
                break;
            }
        }
        // end of the algorithm.
        if (local_k > sqrt(n)) break;
    }
    const auto t2 = std::chrono::high_resolution_clock::now();
    seqTime = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    if(!silent){
        for(i=0; i<n; i++) cout <<"Seq- Prime bool value of " << i << " is: " << primes[i] << endl;
    }
    delete [] primes;
    return seqTime;
}

int main(int argc, char** argv){ // ./a.out 1000 4 silent
    int size = atoi(argv[1]);
    // set thread no and silent arguments
    int threadCount = atoi(argv[2]);
    bool silent = 0;
    if(argv[3] && !strcmp(argv[3],"silent")) silent=1;
    // set data array
    int *data = new int[size];
    for(int i=0; i<size; i++) data[i]=i;

    // start timing for parallel algo.
    const auto t1 = std::chrono::high_resolution_clock::now();
    global_control c(global_control::max_allowed_parallelism,  threadCount);
    std::chrono::duration<double> parallelTime;
    // set primes array
    bool *primes = new bool[size];
    memset(primes, 1, size);

    // set first unmarked number (k) to 2
    int i, local_k = 2;
    primes[0] = primes[1] = 0;
    // algorithm can stop when k > sqrt(size)
    int limit = sqrt(size);

    while(1){
        ParallelApplySieve( data, size, primes , local_k);
        primes[local_k] = 1;
        /*  Here FindNextK parallelization was observed. Instead, sequential search will be employed below.
         *
         *  FindNextK next_k(INT_MAX, primes, limit);
            parallel_reduce(blocked_range<size_t>(local_k+1, limit, next_k);
            local_k=next_k.k;
         */

        // sequential search for finding the next k
        for (i = local_k + 1; i < size; i++) {
            // if unmarked (prime), update k to next prime
            if (primes[i]) {
                local_k = i;
                break;
            }
        }
        // end of the algorithm.
        if (local_k > limit) break;
    }
    // end of the timing
    const auto t2 = std::chrono::high_resolution_clock::now();
    parallelTime = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    cout << "Parallel time: " << parallelTime.count() << "secs" << endl;

    if(!silent){
       for(i=0; i<size; i++) cout <<"Prime bool value of " << i << " is: " << primes[i] << endl;
    }

    // purely sequential run
    cout << "Sequential time: " << SerialSieve( size, silent).count() << endl;

    delete [] data;
    delete [] primes;
    primes = nullptr;
    data = nullptr;
    return 0;
}
