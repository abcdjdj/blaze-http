#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <errno.h>

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

int main(void)
{
    int sockfd, clientSockFD;

    struct sockaddr_in serverSocket, clientSocket;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serverSocket.sin_addr.s_addr = inet_addr(LOCALHOST);
    serverSocket.sin_port = htons(PORTNO);
    serverSocket.sin_family = AF_INET;

    int ret = bind(sockfd, (struct sockaddr*)&serverSocket, sizeof(serverSocket));
    if(ret != 0) {
        fprintf(stderr, "[%s] Error in bind() - %s\n", __func__, strerror(errno));
        return ret;
    }

    listen(sockfd, 5);

    int reqCount = 0;
    while(1) {
        socklen_t size_client = sizeof(clientSocket);

        printf("Waiting for a client..\n");
        clientSockFD = accept(sockfd, (struct sockaddr*)&clientSocket, &size_client);

        printf("Serving Request #%d\n", ++reqCount);
        int ret = fork();
        if(ret == 0) {
            char http_req[2048];
            ssize_t reqSize = read(clientSockFD, http_req, sizeof(http_req));
            http_req[reqSize] = '\0';
            puts(http_req);
            
            /* Handle favicon.ico */
            if(!strncmp("GET /favicon.ico", http_req, strlen("GET /favicon.ico"))) {
                char *iconBuf = NULL;
                ssize_t iconSize = readFile("favicon.ico", &iconBuf);
                write(clientSockFD, iconBuf, iconSize);
            } else {
                char *htmlBuf = NULL;
                ssize_t htmlSize = readFile("index.html", &htmlBuf);

                const char *htmlStatus = "HTTP/1.1 200 OK\r\n\r\n";
                //"Content-Type: text/html; charset=UTF-8\r\n"

                write(clientSockFD, htmlStatus, sizeof(char) * strlen(htmlStatus));
                write(clientSockFD, htmlBuf, htmlSize);
            }

            close(clientSockFD);
            return 0;
        } else {
            close(clientSockFD);
        }
    }
}
