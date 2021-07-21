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
        //cout << "range of chunk is between G/2 and G : " << range.size();
        for( size_t i=range.begin(); i<end; i++ ){
            if(copyArr[i]!=local_k && copyArr[i]%local_k==0) copyPrime[i]=0;
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
    parallel_for(blocked_range<size_t>(0, n, 25000), obj, simple_partitioner()); // grain size
    //parallel_for(blocked_range<size_t>(0, n), obj);   // automatic chunking
}
std::chrono::duration<double> SerialApplySieve( int* a, size_t n , int k) {
    int i;
    int local_k = k;
    std::chrono::duration<double> seqTime;
    bool *primes = new bool[n];
    for( i=0; i<n; i++) primes[i]=1;

    int *data = a;
    bool finish;
    while(1) {
        const auto t1 = std::chrono::high_resolution_clock::now();
        for (i = 0; i < n; i++) {
            if (data[i] != local_k && data[i] % local_k == 0) primes[i] = 0;
        }
        const auto t2 = std::chrono::high_resolution_clock::now();
        seqTime += std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        finish = 1;
        for (i = local_k + 1; i < n; i++) { if (primes[i]) { local_k = i; finish = 0; break; } }    // update k to next prime number
        if (finish || local_k > sqrt(n)) { break; }  // no more primes can be found after this k
    }
    delete [] primes;
    return seqTime;
}

int main(int argc, char** argv){
    bool silent;
    int size = atoi(argv[1]);
    if(argv[2] && !strcmp(argv[2],"silent")) silent=1;

    /* data initializations */
    int *data = new int[size];
    bool *primes = new bool[size];
    for(int i=0; i<size; i++) data[i]=i;      // start with 0, edit at the end.
    for(int i=0; i<size; i++) primes[i]=1;    // all of them are prime(1) at the beginning.
    int i, local_k = 2;                          // initial prime

    bool finish ;
    std::chrono::duration<double> parallelTime;
    while(1){
        const auto t1 = std::chrono::high_resolution_clock::now();
        ParallelApplySieve( data, size, primes , local_k);                       // apply prime sieve for current k
        const auto t2 = std::chrono::high_resolution_clock::now();
        parallelTime += std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        finish = 1;
        for( i=local_k+1; i<size; i++) {if (primes[i]) {local_k=i; finish=0; break;}}    // update k to next prime number
        if(finish || local_k > sqrt(size)) {break;}  // no more primes can be found after this k
    }
    // minor edits
    primes[0] = primes[1] = 0;
    primes[2] = 1;
    cout << "Overall parallel time: " << parallelTime.count() << "secs" << endl;
    cout << "Sequential time: " << SerialApplySieve( data, size , 2).count() << endl;

    if(!silent){
        for(i=0; i<size; i++) cout <<"Prime bool value of " << i << " is: " << primes[i] << endl;
    }
    delete [] data;
    delete [] primes;
    primes = nullptr;
    data = nullptr;
}