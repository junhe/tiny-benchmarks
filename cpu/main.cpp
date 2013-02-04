/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  A micro benchmark to measure the FLOPS and IOPS
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

#define NUM_THREADS_MAX     128
//#define TIMING_METHOD CLOCK_PROCESS_CPUTIME_ID
#define TIMING_METHOD CLOCK_REALTIME

#define OPS_PER_LOOP 20
#define NUM_LOOP     100000000


using namespace std;

int optypenum; // global variable indicate it is calculating iops or flops.

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

// Use the cpu
void *DoOperations(void *t)
{
    unsigned long long i;
    long tid;
    double a = 0.9980;
    double b = -100000.0;

    int ai = 8;
    int bi = 10;

    tid = (long)t;
    
    if ( optypenum == 0 ) {
        printf("Doing floating operations\n");
        for (i = 0; i < NUM_LOOP; i++)
        {
            // Put more operations in a loop so the
            // index operations have smaller effects
            b += a*0.99997 ;
            b += a*0.99997 ;
            b += a*0.99997 ;
            b += a*0.99997 ;
            b += a*0.99997 ;
            b += a*0.99997 ;
            b += a*0.99997 ;
            b += a*0.99997 ;
            b += a*0.99997 ;
            b += a*0.99997 ;
        }
        //printf("Thread %ld done. Result = %e\n",tid, b);
    } else {
        printf("Doing int operations\n");
        for (i = 0; i < NUM_LOOP; i++)
        {
            bi += ai*3 ;
            bi += ai*3 ;
            bi += ai*3 ;
            bi += ai*3 ;
            bi += ai*3 ;
            bi += ai*3 ;
            bi += ai*3 ;
            bi += ai*3 ;
            bi += ai*3 ;
            bi += ai*3 ;
        }       
        //printf("Thread %ld done. Result = %d\n",tid, bi);
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
    int nthreads; // number of threads
    string optype; // type of operations, int or float
    struct timespec time1, time2;
    double totaltime;

    if ( argc != 3 ) {
        printf("Usage: %s nthreads flops/iops\n", argv[0]);
        exit(1);
    }

    nthreads = atoi(argv[1]);
    optype = argv[2];
    
    if ( optype == "flops" ) {
        optypenum = 0;
    } else {
        optypenum = 1;
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

    double opps = (NUM_LOOP/1000000.0)  * OPS_PER_LOOP * nthreads / totaltime;
    printf("Op-Type Total-Time(second) Op-Per-Sec(Million Ops Per Second)\n");
    printf("%s %lf %lf\n", optype.c_str(), totaltime, opps);

    pthread_exit(NULL);

}
