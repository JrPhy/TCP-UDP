// C program for the Server Side
 
// inet_addr
#include <arpa/inet.h>
 
// For threading, link with lpthread
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Semaphore variables
sem_t x, y;
pthread_t tid;
pthread_t readerthreads[100];
int readercount = 0;

int timeout_recv(int fd, int *buf, int len, int nsec) {
    struct timeval timeout;
    timeout.tv_sec = nsec;
    timeout.tv_usec = 0;

    printf("timeout_recv called, timeout %d seconds\n", nsec);

    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt error");
        exit(1);
    }

    int n = recv(fd, buf, len, 0);

    return n;
}
// Reader Function
void* reader(void* args) {
    // Lock the semaphore
    sem_wait(&x);
    readercount++;
 
    if (readercount == 1) sem_wait(&y);
    // Unlock the semaphore
    sem_post(&x);
    
    printf("\n%d reader is inside\n",
           readercount);
 
    int nread, status = 1, kkk = 0;
    int client_request = *((int*)args);
    while(1) {
        ++kkk;
        if(kkk == 10000) {
            nread = timeout_recv(client_request, &status, sizeof(status), 10);
            printf("nread = %d   status = %d\n", nread, status);
            if (nread <= 0) break;
            kkk = 0;
        }
    }
    sleep(1);
    // Lock the semaphore
    sem_wait(&x);
    readercount--;
    if (!readercount) {
        printf("readercount == 0\n");
        sem_post(&y);
    }
 
    // UnLock the semaphore
    sem_post(&x);
 
    printf("\n%d reader is inside\n", readercount);
    pthread_exit(NULL);
}
// Driver Code

int main(int argc, char *argv[]) {
    // Initialize variables
    if (1 == argc) return 0;
    int port;
    char buf[4096], ip[16];
    char *ptr = strtok(argv[1], ":");
    if (ptr == NULL) return 0;
    strcpy(ip, ptr);
    ptr = strtok(NULL, ":");
    port = atoi(ptr);
    if(0 == strcmp(ip, "-1") || !port) {
        printf("Check the IP, PORT\n");
        pthread_exit(NULL);
        return 0;
    }
    int serverSocket, newSocket, i = 0;
    struct sockaddr_in serverAddr;
    sem_init(&x, 0, 1);
    sem_init(&y, 0, 1);
    socklen_t addr_size;
    struct sockaddr_storage serverStorage;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
 
    // Bind the socket to the
    // address and port number.
    bind(serverSocket,
         (struct sockaddr*)&serverAddr,
         sizeof(serverAddr));
 
    // Listen on the socket,
    // with 40 max connection
    // requests queued
    if (listen(serverSocket, 128) == 0) printf("Listening\n");
    else printf("Error\n");
    // Array for thread
    int thread_num = 0, thread_total = 2+1, status = 0, nread = -1, kkk = 0;
    pthread_t tid[thread_total];

    while (1) {
        addr_size = sizeof(serverStorage);
        // Extract the first
        // connection in the queue
        newSocket = accept(serverSocket,
                           (struct sockaddr*)&serverStorage,
                           &addr_size);
                           
        if (pthread_create(&readerthreads[thread_num++], NULL,
                            reader, &newSocket) != 0) {
            // Error in creating thread
            printf("Failed to create thread\n");
            status = -1;
        }
        else {
            printf("create thread sucessfully\n");
            status = 1;
        }
        if( send(newSocket, &status, sizeof(status), 0) < 0) {
            puts("Send failed");
            return 1;
        }
        printf("thread_num out while= %d\n", thread_num);
        printf("status = %d\n", status);
        if (thread_num >= thread_total) {
            // Update thread_num
            thread_num = 0;
            printf("Server is full, please wait\n");
            
            while (thread_num < thread_total) {
                printf("thread_num in while = %d\n", thread_num);
                // Suspend execution of
                // the calling thread
                // until the target
                // thread terminates
                printf("in while\n");
                pthread_join(readerthreads[thread_num++], NULL);
            }
            // Update thread_num
            thread_num = 0;
        }
    }
    return 0;
}

