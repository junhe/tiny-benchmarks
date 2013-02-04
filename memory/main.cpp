/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  A micro benchmark to measure the memory bandwith and latency
 *
 *        Version:  1.0
 *        Created:  02/03/2013 02:41:25 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Jun He
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <vector>
#include <algorithm>

#define NUM_THREADS_MAX     128
//#define TIMING_METHOD CLOCK_PROCESS_CPUTIME_ID
#define TIMING_METHOD CLOCK_REALTIME


using namespace std;


// Global variable
// Don't change once initialized
int optypenum; // global variable indicate it is calculating iops or flops.
long long blocksize;
int nthreads; // number of threads
long long memsize;  // size of memory per thread
long long blockcnt; 
vector<long long> seqvec;

char *frommem[NUM_THREADS_MAX]; // copy from this memory
char *tomem[NUM_THREADS_MAX];  // to this memory

// Calculate the time period
struct timespec diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

// Copy memory
void *DoOperations(void *t)
{
    long tid;

    tid = (long)t;

    
    if ( optypenum == 0 ) {
        // sequential copy
        int i;
        for ( i = 0 ; i < blockcnt ; i++ ) {
            memcpy(tomem[tid] + (i*blocksize), 
                   frommem[tid] + (i*blocksize), 
                   blocksize);
        }
    } else {
        // random copy
        int i;
        int blocki;
        for ( i = 0 ; i < blockcnt ; i++ ) {
            blocki = seqvec[i];
            memcpy(tomem[tid] + (blocki*blocksize), 
                   frommem[tid] + (blocki*blocksize), 
                   blocksize);
        }

    }

    pthread_exit((void*) t);
}

int main (int argc, char *argv[])
{
    pthread_t thread[NUM_THREADS_MAX];
    pthread_attr_t attr;
    int rc;
    long t;
    void *status;
    struct timespec time1, time2;
    double totaltime;


    // do simple check of arguments
    if ( argc != 5 ) {
        printf("Usage: %s nthreads access-type block-size memory-size-per-thread\n", argv[0]);
        printf("access-type: 0 - sequential\n");
        printf("             1 - random\n");
        printf("block-size should be dividor of memory-size-per-thread\n");
        exit(1);
    }

    // initialize parameters
    nthreads = atoi(argv[1]);
    optypenum = atoi(argv[2]);
    blocksize = atol(argv[3]);
    memsize = atol(argv[4]);
    blockcnt = memsize/blocksize;

    long long i;
    for ( i = 0 ; i < blockcnt ; i++ ) {
        seqvec.push_back(i);
    }

    srand ( unsigned ( time(0) ) );
    random_shuffle ( seqvec.begin(), seqvec.end() );
    /*  
    for ( i = 0 ; i < 100 ; i++ ) {
        cout << seqvec[i] << ",";
    }
    */

    // initialize memory
    for ( t = 0 ; t < nthreads ; t++ ) {
        frommem[t] = (char *) malloc(memsize); 
        if ( frommem[t] == NULL ) {
            printf("error when allocating memmory.\n");
            exit(1);
        }
        tomem[t] = (char *) malloc(memsize); 
        if ( tomem[t] == NULL ) {
            printf("error when allocating memmory.\n");
            exit(1);
        }
    }

    // Init pthread
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    clock_gettime(TIMING_METHOD, &time1); // get start time
    for(t=0; t<nthreads; t++) {
        rc = pthread_create(&thread[t], &attr, DoOperations, (void *)t); 
        if (rc) {
            exit(-1);
        }
    }

    /* Free attribute and wait for the other threads */
    pthread_attr_destroy(&attr);
    for(t=0; t<nthreads; t++) {
        rc = pthread_join(thread[t], &status);
        if (rc) {
            exit(-1);
        }
    }

    clock_gettime(TIMING_METHOD, &time2); // get end time
    totaltime = diff(time1,time2).tv_sec + diff(time1,time2).tv_nsec/1000000000.0;

    printf("Copied-Size Total-Time(second) Bandwidth (MB/s) Latency(ms) Block-Size Block-Count\n");
    printf("%lld %lf %lf %lf %lld %lld GREPMARKER\n", memsize, totaltime, 
            (memsize/(1024*1024))/totaltime, (1000.0*totaltime/blockcnt),
            blocksize, blockcnt);

    pthread_exit(NULL);

}

