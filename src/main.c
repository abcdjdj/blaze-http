#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <errno.h>

#include "C-Thread-Pool/thpool.h"

#define PORTNO 8080
#define LOCALHOST "127.0.0.1"

ssize_t readFile(char *fileName, char **buf)
{
    FILE *fp = fopen(fileName, "r");
    if(!fp) {
        fprintf(stderr, "[%s] Error opening file %s\n", __func__, fileName);
        return 0;
    }
    
    fseek(fp, 0, SEEK_END);
    ssize_t fileSize = ftell(fp);

    *buf = (char *)malloc(sizeof(char) * fileSize);
    if(!*buf) {
        fprintf(stderr, "[%s] Error in malloc() while reading file %s\n", __func__, fileName);
        goto CLEANUP;
    }

    rewind(fp);
    fread(*buf, 1, fileSize, fp);

    CLEANUP:
    fclose(fp);
    return fileSize;
}


void serviceRequest(void *threadID)
{
    fprintf(stdout, "Hello from thead %d\n", *(int *)threadID);

    // TODO: Service code here

    fprintf(stdout, "Terminating %d\n", *(int *)threadID);
    free(threadID);
}

void threadPoolInit(threadpool *pool)
{
    *pool = thpool_init(4); // TODO: Get nproc?
    if(pool == NULL) {
        fprintf(stderr, "[%s] Error initializing thread pool\n", __func__);
        return;
    }
}

int socketInit(struct sockaddr_in *serverSocket, int *sockFD)
{
    *sockFD = socket(AF_INET, SOCK_STREAM, 0);
    serverSocket->sin_addr.s_addr = inet_addr(LOCALHOST);
    serverSocket->sin_port = htons(PORTNO);
    serverSocket->sin_family = AF_INET;

    int ret = bind(*sockFD, (struct sockaddr*)serverSocket, sizeof(struct sockaddr_in));
    if(ret != 0) {
        fprintf(stderr, "[%s] Error in bind() - %s\n", __func__, strerror(errno));
        return ret;
    }

    listen(*sockFD, 5);
}

int main(void)
{
    struct sockaddr_in serverSocket;
    int sockFD;
    socketInit(&serverSocket, &sockFD);


    threadpool pool;
    threadPoolInit(&pool);

    struct sockaddr_in clientSocket;
    int reqCount = 0;
    while(1) {
        socklen_t size_client = sizeof(struct sockaddr_in);

        printf("Waiting for a client..\n");
        int clientSockFD = accept(sockFD, (struct sockaddr*)&clientSocket, &size_client);

        int *threadID = (int *)malloc(sizeof(int));
        *threadID = ++reqCount;
        thpool_add_work(pool, serviceRequest, threadID);
    }
}
