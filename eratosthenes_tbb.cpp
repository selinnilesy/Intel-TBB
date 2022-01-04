//
// Created by Selin Yıldırım on 20.07.2021.
//
#include "tbb/tbb.h"
#include <iostream>
#include <stdlib.h>     /* atoi */
#include <cstring>
#include <numeric>
#include <chrono>
#include <ctime>
#include <cmath>

using namespace tbb;
using namespace std;

class ApplySieve {
    int *const arr;
    const int k;
public:
    bool* isPrime;
    void operator()( const blocked_range<size_t>& range ) const {
        int *copyArr = arr;          //  use registers instead of accessing to memory: performance improvement
        bool *copyPrime = isPrime;
        int local_k = k;
        size_t end = range.end();

        for( size_t i=range.begin(); i<end; i++ ){
            if(copyArr[i]!=local_k && copyArr[i]%local_k==0)
                copyPrime[i]=0;
        }
    }
    ApplySieve(int *data, int k_init, bool* isPrimeInit) : arr(data), k(k_init), isPrime(isPrimeInit) {}

    /*
     * parallel_reduce together with a split+join might be used to accumulate a prime vector returned froom each split.

     ApplySieve( ApplySieve& x, split ) : arr(x.arr), k(x.k), isPrime(x.isPrime) {}

    void join( const ApplySieve& y ) {
       this->isPrime = y.isPrime;
    }
    */
};

void ParallelApplySieve( int* a, size_t n, bool* primes , int local_k) {
    ApplySieve obj(a, local_k, primes);
    //parallel_for(blocked_range<size_t>(0, n, 25000), obj, simple_partitioner()); // grain size
    parallel_for(blocked_range<size_t>(0, n), obj);   // automatic chunking
}

std::chrono::duration<double> SerialApplySieve( int* a, size_t n , int k) {
    const auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> seqTime;

    int i, local_k = k;
    int *data = a;

    bool *primes = new bool[n];
    memset(primes, 1, n);

    while(1) {
        // mark composite as not-prime
        for (i = 0; i < n; i++) {
            if (data[i] != local_k && data[i] % local_k == 0)
                primes[i] = 0;
        }

        for (i = local_k + 1; i < n; i++) {
            if (primes[i]) {
                local_k = i; break;
            }
        }    // update k to next prime number

        if (local_k > sqrt(n)) { break; }  // no more primes can be found after this k
    }

    const auto t2 = std::chrono::high_resolution_clock::now();
    seqTime = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

    primes[0] = primes[1] = 0;
    primes[2] = 1;

    delete [] primes;
    return seqTime;
}

int main(int argc, char** argv){
    bool silent=0;
    int size = atoi(argv[1]);
    int threadCount = atoi(argv[2]);
    if(argv[3]  && !strcmp(argv[3],"silent")) silent=1;

    /* data initializations */
    int *data = new int[size];
    for(int i=0; i<size; i++) data[i]=i;      // start with 0, edit at the end.

    const auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> parallelTime;
    bool *primes = new bool[size];
    for(int i=0; i<size; i++) primes[i]=1;    // all of them are prime(1) at the beginning.
    int i, local_k = 2;                          // initial prime

    global_control c(global_control::max_allowed_parallelism,  threadCount);
    while(1){
        ParallelApplySieve( data, size, primes , local_k);                       // apply prime sieve for current k
        for( i=local_k+1; i<size; i++) {if (primes[i]) {local_k=i; break;}}    // update k to next prime number
        if(local_k > sqrt(size)) {break;}  // no more primes can be found after this k
    }
    const auto t2 = std::chrono::high_resolution_clock::now();
    parallelTime = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

    // minor edits
    primes[0] = primes[1] = 0;
    primes[2] = 1;

    cout << "Overall parallel time: " << parallelTime.count() << "secs" << endl;
    //cout << "Sequential time: " << SerialApplySieve( data, size , 2).count() << endl;

    if(!silent){
        for(i=0; i<size; i++) cout <<"Prime bool value of " << i << " is: " << primes[i] << endl;
    }
    delete [] data;
    delete [] primes;
    primes = nullptr;
    data = nullptr;
}
