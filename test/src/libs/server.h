#include "utils/json.c"

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

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

/*
creates a struct Server
*/
struct Server ServerInit(int domain,int service,int protocol,u_long interface,int port,int backlog);

/*
creates a struct Server with default values, only changing the port and amount of backlog, 
you should use this rather than ServerInit in most cases
*/
struct Server ServerDefaultInit(int port,int backlog);

/*
runs the server, it will loop forever
*/
void ServerRun(struct Server* server);

/*
creates a http header
code is the return code of the request (200,404,500,...) and 
contentLength is the length of the content
headers is a json containing headers for the http request
the headers must be of the form "Header-Name":"headerValue", exemple: "Content-Type":"text/html"
*/
String ServerMakeHeader(int code,int contentLength,String headers,struct Arena* arena);

/*
awaits an http request, returns the client socket that is needed to get the request content and respond to the request
*/
int ServerAwaitConnection(struct Server* server);

/*
returns the http request as a String from a clientSocket
*/
String ServerGetRequest(int clientSocket,int maxRequestSize,struct Arena* arena);

/*
used inside of a c page to parse a request of the form ?arg1=value1&arg2=value2&... to match html forms, it automatically checks for POST or GET
*/
Hashmap ServerParseRequest(char** argv,struct Arena* arena);

/*
used inside of a c page to respond to a request
*/
void ServerRespond(int code,String headers,String content,struct Arena* arena);

/*
used to get the ip that sent the request, not to be confused with the clients ip, this will most likely be a router
*/
struct sockaddr_in ServerGetSenderIp(int clientSocket);

/*
writes str to the response socket, this is additive
*/
void ServerSocketWriteTo(int clientSocket,String str,struct Arena* arena);

/*
closes the socket
*/
void ServerSocketClose(int clientSocket);

/*
calculates how many layers deep a path is, this is mainly used to check if it is negative to prevent requesting an off limits file
*/
int ServerPathDepth(String path);

/*
return a String of the requests method(POT,GET,...)
*/
String ServerRequestMethod(String request,struct Arena* arena);

/*
returns a String of the requests path
*/
String ServerRequestPath(String request,struct Arena* arena);

/*
returns the content without the headers of the request
*/
String ServerRequestContent(String request,struct Arena* arena);

/*
return a String with everything after and including the ? character
*/
String ServerStrip(String path,struct Arena* arena);

/*
returns the extension of the path given(html,css,c,png,...)
*/
String ServerPathExtension(String path,struct Arena* arena);

/*
parses a GET request of the form ?arg1=value1&arg2=value2&... to match html forms
*/
Hashmap ServerParseGET(char* rawurl,struct Arena* arena);

/*
parses a POST request of the form ?arg1=value1&arg2=value2&... to match html forms
*/
Hashmap ServerParsePOST(char* rawcontent,struct Arena* arena);