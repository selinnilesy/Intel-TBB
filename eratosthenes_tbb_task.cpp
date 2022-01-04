#include "tbb/tbb.h"
#include <iostream>
#include <stdlib.h>     /* atoi */
#include <cstring>
#include <numeric>
#include <chrono>
#include <cmath>
#include <cassert>

using namespace tbb;
using namespace std;

void SerialStrike(int , int, int, bool*);

class PrimeTask: public task {
public:
    bool* primes;
    int k, begin, end, cutoff;
    PrimeTask( int k_ , bool* primes_, int begin_, int end_, int cutoff_ ) :
            k(k_), primes(primes_), begin(begin_), end(end_), cutoff(cutoff_)
    {}
    task* execute() { // Overrides virtual function task::execute
        if( (end-begin)< cutoff) {
            SerialStrike(begin, end, k, primes);
        }
        else {
            int middle = this->begin + (this->end - this->begin + this->k - 1) / 2;
            middle = middle / this->k * this->k;
            PrimeTask& b = *new( allocate_child() ) PrimeTask(k, primes, middle, end, cutoff);
            this->end = middle;
            PrimeTask& a = *new( allocate_child() ) PrimeTask(k, primes, begin, middle, cutoff);
            // Set ref_count to "two children plus one for the wait".
            set_ref_count(3);
            // Start a running.
            spawn(a);
            // Start b running and wait for all children (a and b).
            spawn_and_wait_for_all(b);
        }
        return NULL;
    }
};
void SerialStrike( int begin, int end,  int k, bool *primes ) {
    size_t i = begin;
    size_t l_end = end;
    int local_k = k;
    bool *copyPrime = primes;
    for( ; i<l_end;  i+=local_k ){
        copyPrime[i] = 0;
        //cout << i << "has been stroke." << endl;
    }
}

void ParallelPrime(int size, bool silent, int cores, int cutoff) {
    const auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> parallelTime;

    int k = 2;
    bool *primes = new bool[size];
    memset(primes, 1, size);
    primes[0]=primes[1]=0;

    bool flag;
    int limit = sqrt(size);

    while(1){
        //cout << k << " is being processed. " << endl;
        PrimeTask& a = *new(task::allocate_root()) PrimeTask( k, primes, pow(k,2), size, cutoff);
        task::spawn_root_and_wait(a);
        flag=0;
        for(int i=k+1; i<limit; i++){
            if(primes[i]){ k=i; flag=1; break;}
        }
        if(!flag || k>sqrt(size)) break;
    }

    const auto t2 = std::chrono::high_resolution_clock::now();
    parallelTime = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    cout << "Parallel time: " << parallelTime.count() << "secs" << endl;

    if(!silent){
        for(int i=0; i<size; i++) cout <<  "Prime val of " << i << " is: " << primes[i] << endl;
    }
    delete [] primes;
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

int main(int argc, char** argv){    // ./a.out 1000 cores cutoff silent
    int size = atoi(argv[1]);
    int cores = atoi(argv[2]);
    int cutoff = atoi(argv[3]);
    bool silent=0;

    if(argv[4] && !strcmp(argv[4],"silent")) silent=1;
    ParallelPrime( size, silent, cores, cutoff);

    // purely sequential run
    //cout << "Sequential time: " << SerialSieve( size, silent).count() << endl;

}
