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


//#define NUM_THREADS_MAX     128
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
    exit(1);
}

void *DoOperations_TCP(void *t)
{
    int socketfd;

    socketfd = (int)t;
    int n;
    char buffer[256];

    bzero(buffer,256);
    n = read(socketfd,buffer,255);
    if (n < 0) error("ERROR reading from socket");
    printf("Here is the message: %s\n",buffer);
   
    close(socketfd);

    pthread_exit((void*) t);
}

int main (int argc, char *argv[])
{
    int rc;
    string fpath;
    string protocol_type_str;

    // for network
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    // do simple check of arguments
    if ( argc != 5 ) {
        printf("Usage: %s nthreads protocol-type buffer-size"
               " portno\n", argv[0]);
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
    portno = atoi(argv[4]);

    if ( protocoltypenum == PROTOCOL_TCP ) {
        // TCP
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) 
            error("ERROR opening socket");
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        if (bind(sockfd, (struct sockaddr *) &serv_addr,
                    sizeof(serv_addr)) < 0) 
            error("ERROR on binding");
        listen(sockfd,5);
        clilen = sizeof(cli_addr);

        while (1) {
            newsockfd = accept(sockfd, 
                    (struct sockaddr *) &cli_addr, 
                    &clilen);
            if (newsockfd < 0) 
                error("ERROR on accept");
            pthread_t tmpid;
            rc = pthread_create(&tmpid, NULL, DoOperations_TCP, (void *)newsockfd); 
            if (rc) {
                exit(-1);
            }
        }
        close(sockfd);
    } else {
        // UDP
        sockfd=socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) 
            error("ERROR opening socket");
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        if (bind(sockfd, (struct sockaddr *) &serv_addr,
                    sizeof(serv_addr)) < 0) 
            error("ERROR on binding");
        clilen = sizeof(cli_addr);

        char buf[1024];
        while (1) {
            int n;
            bzero(buf,256);
            printf("Waiting for data...\n");
            n = recvfrom(sockfd, buf, 1024,0,(struct sockaddr *)&cli_addr,&clilen);
            if (n < 0) error("recvfrom");
            write(1,"Received a datagram: ",21);
            write(1,buf,n);
        }

        close(sockfd);
    }
    pthread_exit(NULL);
}

