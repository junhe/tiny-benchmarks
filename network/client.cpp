/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  A micro benchmark to measure the network bandwith and latency
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
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define NUM_THREADS_MAX     128
//#define TIMING_METHOD CLOCK_PROCESS_CPUTIME_ID
#define TIMING_METHOD CLOCK_REALTIME


using namespace std;


// Global variable
// Don't change once initialized
int protocoltypenum;
#define PROTOCOL_TCP 0
#define PROTOCOL_UDP 1

long long buffersize;
int nthreads; // number of threads
long long datasize_per_thread;  // size of memory per thread
string ipstr;

char *datamem[NUM_THREADS_MAX]; // copy from this memory

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

//  
void *DoOperations(void *t)
{
    long tid;

    tid = (long)t;
    off_t bytes = 0;

    if ( protocoltypenum == PROTOCOL_TCP ) {
    } else {
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
    string fpath;
    string protocol_type_str;

    // do simple check of arguments
    if ( argc != 6 ) {
        printf("Usage: %s nthreads protocol-type buffer-size data-size-per-thread"
               " ip\n", argv[0]);
        printf("protocol-type: 0 - tcp\n");
        printf("               1 - udp\n");
        printf("buffer-size should be dividor of data-size-per-thread\n");
        exit(1);
    }

    // initialize parameters
    nthreads = atoi(argv[1]);
    protocoltypenum = atoi(argv[2]);
    if ( protocoltypenum == PROTOCOL_TCP ) {
        protocol_type_str = "TCP";
    } else {
        protocol_type_str = "UDP";
    }

    buffersize = atol(argv[3]);
    datasize_per_thread = atol(argv[4]);
    ipstr = argv[5];


    // initialize memory
    for ( t = 0 ; t < nthreads ; t++ ) {
        datamem[t] = (char *) malloc(buffersize); 
        if ( datamem[t] == NULL ) {
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

    printf("    File-Size Total-Time(second) Bandwidth(MB/s) Latency(ms) Block-Size Block-Count"
           "   Rwmode Access-Type\n");


    pthread_exit(NULL);
}

