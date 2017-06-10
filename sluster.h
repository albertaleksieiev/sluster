#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>

#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H


typedef void(*onmessage_callback)(int, char*, struct sockaddr *, socklen_t);

class ServerSocket{
public:
    char* port;

    onmessage_callback onmessage;
    ServerSocket(const char* port, onmessage_callback onmessage){
        this->port = new char[strlen(port)];
        strcpy(this->port, port);

        this->onmessage = onmessage;
    }
    ~ServerSocket(){
        delete this->port;
    }

    int start(){
        int sockfd;
        struct addrinfo hints, *servinfo, *p;
        int rv;
        int numbytes;
        struct sockaddr_storage their_addr;
        int MAXBUFLEN = 65536;

        char buf[MAXBUFLEN];
        socklen_t addr_len;
        char s[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE; // use my IP

        if ((rv = getaddrinfo(NULL, this->port, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }

        // loop through all the results and bind to the first we can
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                                 p->ai_protocol)) == -1) {
                perror("listener: socket");
                continue;
            }

            if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                perror("listener: bind");
                continue;
            }

            break;
        }

        if (p == NULL) {
            fprintf(stderr, "listener: failed to bind socket\n");
            return 2;
        }

        freeaddrinfo(servinfo);


        addr_len = sizeof their_addr;

        while(true) {
            if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                                     (struct sockaddr *) &their_addr, &addr_len)) == -1) {
                perror("recvfrom");
                exit(1);
            }


            onmessage(sockfd,buf,(struct sockaddr *) &their_addr, (socklen_t) addr_len);

            buf[numbytes] = '\0';
        }
        close(sockfd);
    }
};

void* socket_func(void* server_socket){
    ServerSocket* serverSocket = (ServerSocket*)server_socket;
    serverSocket->start();
}
class CalculationSocketManager{
    ServerSocket* serverSocket;
    pthread_t socket_thread;

public:
    CalculationSocketManager(char* port, onmessage_callback onmessage){
        serverSocket = new ServerSocket(port, onmessage);
    }
    ~CalculationSocketManager(){
        delete serverSocket;
    }
    int start(void(*calc_func)(void*) ){
        printf("starting UDP server on port %s\n", this->serverSocket->port);
        if(pthread_create(&socket_thread, NULL,socket_func, this->serverSocket)) {
            fprintf(stderr, "Error creating socket thread\n");
            return 1;
        }
        //Start calculation
        calc_func(NULL);
    }
};
#endif
