//
// Created by Selin Yıldırım on 20.07.2021.
//
#include "tbb/tbb.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>     /* atoi */
#include <cstring>
#include <numeric>
#include <chrono>
#include <cmath>
#include <cassert>

using namespace tbb;
using namespace std;

/* This class splits the iteration space according to the current stride and grainsize.
 * Range  is selected according to the first number the stride can divide without a remainder.
 * */
class SieveRange {
    size_t my_stride, my_grainsize;
    size_t my_begin,my_end;
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
        size_t middle = r.my_begin + (r.my_end - r.my_begin + r.my_stride - 1) / 2;
        middle = middle / my_stride * my_stride;
        my_begin = middle;
        r.my_end = middle;
        assert(assert_okay());
        assert(r.assert_okay());
    }
    size_t begin() const {
        return my_begin;
    }
    size_t end() const {
        return my_end;
    }
    SieveRange(size_t begin, size_t end, size_t stride, size_t grainsize)
            : my_begin(begin),
              my_end(end),
              my_stride(stride),
              my_grainsize(grainsize < stride ? stride : grainsize) {
        assert(assert_okay());
    }
};
/* This is the innermost class functioning as Sieve of Eratosthenes.
 * Stride k is used to strike the composite numbers of each range.
 * Primes up to limit is computed during initialization.
 * Then, the rest of data is struck each time the window proceeds to right with odd numbers.
 * An odd number that is not prime is never used as a striker, being struck.
 * This exploits homogenous distribution of even numbers amongst the data range.
 * Meanwhile data decomposition may happen via splitter of SieveRange class.
 * */
class ApplySieve {
    bool* is_composite;
    size_t *striker, *primes;
public:
    size_t limit;
    size_t primeCount;
    inline size_t strike(size_t start, size_t limit, size_t stride) {
        bool* curr_data = is_composite;
        size_t i=start;
        for (;  i < limit; i += stride)
            curr_data[i] = 1;
        return i;   // where we left in the data array for that k's turn.
    }

    // natural constructor. only finds primes upto limit.
    ApplySieve(size_t n) :  primeCount(0), limit(   ((size_t)  sqrt(double(n))) + (((size_t) sqrt(double(n))) & 1)    ) {
        is_composite = new bool[limit / 2];
        striker = new size_t[limit / 2];
        primes = new size_t[limit / 2];
        memset(is_composite, 0, limit / 2);

        // start from odd numbers, skip 1.
        size_t half_limit = limit/2;
        for (size_t k = 3; k<limit; k+=2) {
            size_t temp = k/2;
            // do not visit if already marked as composite.
            if (!is_composite[temp]) {
                // striker is the next and first non-struck number corresponding to this k.
                // (an index into window)
                striker[primeCount] = strike(k/2, half_limit, k);
                primes[primeCount++] = k;   // save the prime
            }
        }
    }

    // Sieve controller uses this function. Finds primes in the window.
    size_t find_primes_in_window(size_t start, size_t window_size) {
        size_t i=0;
        bool* curr_data = is_composite;
        // reset the composite array for re-use.
        size_t half_window = window_size/2;
        memset(curr_data, 0, half_window);

        // considering where we left last time, find the new offset and strike the new half window.
        for (i=0;  i<primeCount; ++i)
            striker[i] = strike(striker[i] - limit/2,  half_window, primes[i]);

        size_t tempCount = 0;
        for (i=0; i< half_window; ++i) {
            if (!curr_data[i]) {
                ++tempCount;
                //printf("%ld\n", long(start + 2 * i + 1));
            }
        }
        return tempCount;
    }

    // split constructor.
    ApplySieve(const ApplySieve& my, tbb::split)
            : primeCount(my.primeCount),
              limit(my.limit),
              primes(my.primes),
            // do not lose primes but empty the computing arrays.
              is_composite(nullptr),
            // striker will be adapted to its new value via copyStrikerDuringSplit().
              striker(nullptr) {}

    void copyStrikerDuringSplit(size_t start) {
        assert(start >= 1);
        is_composite = new bool[limit / 2];
        striker = new size_t[limit / 2];
        for (size_t i=0; i<primeCount; ++i) {
            size_t prime = primes[i];
            size_t offset = (start - 1) / prime * prime % limit;
            striker[i] = (offset & 1 ? offset + 2 * prime : offset + prime) / 2;
            assert(limit / 2 <= striker[i]);
        }
    }

    bool isInitialized(){ return is_composite; }

    void move(ApplySieve& other) {
        swap(striker, other.striker);
        swap(is_composite, other.is_composite);
        // other.my_factor is a shared pointer that was copied by the splitting constructor.
        // Set it to nullptr to prevent premature deletion by the destructor of ~other.
        assert(primes == other.primes);
        other.primes = nullptr;
    }

    ~ApplySieve() {
        delete [] striker;
        delete [] primes;
        delete [] is_composite;
    }
};

class SieveController {
public:
    // initial primes are set during the construction of ApplySieve object.
    ApplySieve initialPrimes;
    size_t primeCount;

    // size of the data array
    SieveController(size_t size) : initialPrimes(size), primeCount(0) {}

    void operator()(const SieveRange& range) {
        size_t limit = initialPrimes.limit;

        //  if just splitted the data array
        if (!initialPrimes.isInitialized()) {
            initialPrimes.copyStrikerDuringSplit(range.begin());
        }

        size_t window = limit;
        size_t i = range.begin();
        size_t end = range.end();
        for (  ;       i < end;     i+=window) {
            assert(i % initialPrimes.limit == 0);
            if (i + window > end)
                window = end - i;
            this->primeCount += initialPrimes.find_primes_in_window(i, window);
        }
    }

    SieveController(SieveController& other, tbb::split)
    // invoke the split of inner class so its lists are transferred to data sets
            : initialPrimes(other.initialPrimes, tbb::split()),
              primeCount(0) {}

    void join(SieveController& other) {
        // accumulate prime count and obtain previous datasets' lists
        this->primeCount += other.primeCount;
        initialPrimes.move(other.initialPrimes);
    }
};

/*int  SerialSieve( size_t n) {
    size_t i, j, local_k = 2;
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

    delete [] primes;
    return 1;
}*/

size_t SerialCountPrimes(size_t n) {
    // Two is special case
    size_t count = n >= 2;
    if (n >= 3) {
        ApplySieve multiples(n);
        count += multiples.primeCount;
        size_t window_size = multiples.limit;
        for (size_t j = multiples.limit; j <= n; j += window_size) {
            if (j + window_size > n + 1)
                window_size = n + 1 - j;
            count += multiples.find_primes_in_window(j, window_size);
        }
    }
    return count;
}

size_t ApplyParallelPrime(size_t size, size_t threadCount, size_t grainsize, bool silent){ // ./a.out 1000 4 silent
    global_control c(global_control::max_allowed_parallelism,  threadCount);
    size_t totalCount=1;

    SieveController controller(size);
    parallel_reduce(SieveRange(controller.initialPrimes.limit, size, controller.initialPrimes.limit, 1000),
                    controller,
                    simple_partitioner());

    totalCount += controller.primeCount;
    return totalCount;
}

int main(int argc, char**argv) {  // ./reduce_optimized 10^6 silent
    std::istringstream arg_size( argv[1] );
    std::istringstream thread_size( argv[2] );
    bool silent = 0;
    size_t size, threadCount;
    arg_size >> size;
    thread_size >> threadCount;
    //if(argv[3] && !strcmp(argv[3],"silent")) silent=1;

    std::chrono::duration<double> seqTime;

    // Try different numbers of threads
    //for (int threadCount = 1; threadCount <= 128 ; threadCount*=2) {
        // repeat and take the average
        std::chrono::duration<double> parallelTime;
        size_t count = 0;

        if (threadCount==1) {
            const auto t1 = std::chrono::high_resolution_clock::now();
            count = SerialCountPrimes(size);
            const auto t2 = std::chrono::high_resolution_clock::now();
            seqTime = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        }
        else {
            const auto t1 = std::chrono::high_resolution_clock::now();
            count = ApplyParallelPrime(size, threadCount, 1000, silent);
            const auto t2 = std::chrono::high_resolution_clock::now();
            parallelTime =  std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        }

        if (threadCount>1){
            std::cout << "#primes from [2.." << size << "] = " << (long) count << " ("
                      << parallelTime.count() << " sec with ";
            std::cout << threadCount << "-way parallelism";
            cout << " - Speedup: " <<  seqTime.count()/parallelTime.count() << endl;
        }
        else{
            std::cout << "#primes from [2.." << size << "] = " << (long) count << " ("
                      <<  seqTime.count() << " sec with ";
            std::cout << "serial code";
        }

        std::cout << ")\n";

    //}
    return 0;
}
