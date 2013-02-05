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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

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
int portno;

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

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

//  
void *DoOperations_UDP(void *t)
{
    //long tid;
    int sock, n;
    unsigned int length;
    struct sockaddr_in server;
    struct hostent *hp;
    char *buffer;

    sock= socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) error("socket");

    server.sin_family = AF_INET;
    hp = gethostbyname(ipstr.c_str());
    if (hp==0) error("Unknown host");

    bcopy((char *)hp->h_addr, 
            (char *)&server.sin_addr,
            hp->h_length);
    server.sin_port = htons(portno);
    length=sizeof(struct sockaddr_in);

    buffer = (char *)malloc( buffersize );
   
    int i;
    int bufcnt = datasize_per_thread/buffersize; // how many buffer
                                                 // you need to send
    for ( i = 0 ; i < bufcnt ; i++ ) {
        n=sendto(sock,buffer,
             buffersize,0,(const struct sockaddr *)&server,length);
        if (n < 0) error("Sendto");
    }

    free(buffer);

    close(sock);
    pthread_exit((void*) t);
}

void *DoOperations_TCP(void *t)
{
    //long tid;
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char *buffer;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(ipstr.c_str());
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");


    buffer = (char *)malloc( buffersize );
   
    int i;
    int bufcnt = datasize_per_thread/buffersize; // how many buffer
                                                 // you need to send
    for ( i = 0 ; i < bufcnt ; i++ ) {
        n = write(sockfd,buffer,buffersize);
        printf("sent %d bytes.\n", n);
        if (n < 0) 
             error("ERROR writing to socket");
    }
    close(sockfd);

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
    if ( argc != 7 ) {
        printf("Usage: %s nthreads protocol-type buffer-size data-size-per-thread"
               " ip portno\n", argv[0]);
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
    portno = atoi(argv[6]);

    cout << "Port :" << portno << endl;

    // Init pthread
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    clock_gettime(TIMING_METHOD, &time1); // get start time

    for(t=0; t<nthreads; t++) {
        if ( protocoltypenum == PROTOCOL_TCP ) {
            rc = pthread_create(&thread[t], &attr, DoOperations_TCP, (void *)t); 
        } else {
            rc = pthread_create(&thread[t], &attr, DoOperations_UDP, (void *)t); 
        }

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
    int totalsize = datasize_per_thread * nthreads;

    printf("Total-Data-Size Total-Time(second) Bandwidth(MB/s) Latency(ms)"
           " Protocol\n");
    printf("%15d %18lf %15lf %11lf %8s GREPMAKER\n",
            totalsize, totaltime, (totalsize/(1024.0*1024.0))/totaltime, 
            totaltime*1000.0/(totalsize/buffersize), protocol_type_str.c_str());

    pthread_exit(NULL);
}

