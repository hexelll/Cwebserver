#include "utils/json.c"

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

struct Server{
    int domain;
    int service;
    int protocol;
    u_long interface;
    int port;
    int backlog;
    int socket;

    struct sockaddr_in address;
    void (*onLaunch)(struct Server*);
    void (*onAwait)(struct Server*);
    void (*onRequest)(struct Server*,String);
    void (*onGET)(struct Server*,String);
    void (*onPOST)(struct Server*,String);
};

struct Server ServerInit(int domain,int service,int protocol,u_long interface,int port,int backlog);

struct Server ServerDefaultInit(int port,int backlog);