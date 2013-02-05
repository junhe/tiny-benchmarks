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
int optypenum;
#define TYPESEQUENTIAL 0
#define TYPERANDOM     1
long long blocksize;
int nthreads; // number of threads
long long datasize_per_thread;  // size of memory per thread
long long blockcnt_whole_file; 
long long blockcntperthread;
vector<long long> seqvec;
int rwmode;
#define MODEREAD   0
#define MODEWRITE  1 
int fd;

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

// Copy memory
void *DoOperations(void *t)
{
    long tid;

    tid = (long)t;
    off_t bytes = 0;

    if ( optypenum == TYPESEQUENTIAL ) {
        // sequential copy
        long long i;
        long long base;
        base = datasize_per_thread * tid;
        for ( i = 0 ; i < blockcntperthread ; i++ ) {
            if ( rwmode == MODEREAD ) {
                bytes += pread(fd, datamem[tid], blocksize, base + i*blocksize);
            } else {
                bytes += pwrite(fd, datamem[tid], blocksize, base + i*blocksize);
            } 
        }
    } else {
        // random copy
        long long i;
        long long base;
        base = blockcntperthread * tid;
        long long blocki;
        for ( i = 0 ; i < blockcntperthread ; i++ ) {
            blocki = seqvec[base+i];
            if ( rwmode == MODEREAD ) {
                bytes += pread(fd, datamem[tid], blocksize, blocki*blocksize);
            } else {
                bytes += pwrite(fd, datamem[tid], blocksize, blocki*blocksize);
            } 
        }
    }
    printf( "%lld bytes accessed by Thread %ld\n", bytes, tid ); 
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
    string rwmodestr;
    string optypestr;

    // do simple check of arguments
    if ( argc != 7 ) {
        printf("Usage: %s nthreads access-type block-size data-size-per-thread"
               " file-path rwmode\n", argv[0]);
        printf("access-type: 0 - sequential\n");
        printf("             1 - random\n");
        printf("mode: 0 - read\n");
        printf("      1 - write\n");
        printf("block-size should be dividor of data-size-per-thread\n");
        exit(1);
    }

    // initialize parameters
    nthreads = atoi(argv[1]);
    optypenum = atoi(argv[2]);
    if ( optypenum == TYPESEQUENTIAL ) {
        optypestr = "Sequential";
    } else {
        optypestr = "Random";
    }

    blocksize = atol(argv[3]);
    datasize_per_thread = atol(argv[4]);
    blockcnt_whole_file = (datasize_per_thread*nthreads)/blocksize; // this is the 
                                                    // total number of blocks
                                                    // in the whole file
    blockcntperthread = datasize_per_thread/blocksize;
    fpath = argv[5];
    rwmode = atoi(argv[6]); 
    if ( rwmode == MODEREAD ) {
        rwmodestr = "read";
    } else {
        rwmodestr = "write";
    }

    fd = open(fpath.c_str(), O_RDWR|O_CREAT, 0666); 
    if ( fd == -1 ) {
        perror( "Cannot open file:" );
        exit(1);
    }


    // Shuffle sequential number to get random sequences
    long long i;
    for ( i = 0 ; i < blockcnt_whole_file ; i++ ) {
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
        datamem[t] = (char *) malloc(blocksize); 
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
    printf("%13lld %18lf %15lf %10f %10lld %10lld %10s %11s\n",
           datasize_per_thread*nthreads, totaltime, (datasize_per_thread/1000000.0)*nthreads/totaltime, 
           totaltime*1000.0/blockcnt_whole_file, blocksize, blockcnt_whole_file, 
           rwmodestr.c_str(), optypestr.c_str()); 

    close(fd);
    pthread_exit(NULL);

}

