#include "server.h"

#define MAXREQUESTSIZE 10000

#define DEBUG 0

struct Arena globalArena;
Hashmap* defaultHeaders;

String ServerMapToHeaders(Hashmap* map,struct Arena* arena) {
    MapKeys mapKeys = HashmapKeys(map);
    struct Arena scratch = ArenaCreate(1024);
    String headers = StringFrom("",&scratch);
    for(int i=0;i<mapKeys.size;i++) {
        char* key = mapKeys.keys[i];
        headers = StringFormat(&scratch,StringFrom("%S%s:%S\n",&scratch),headers,key,*(String*)HashmapGet(map,key));
    }
    headers = StringCpy(headers,arena);
    ArenaDelete(&scratch);
    return headers;
}

void ServerInitDefault() {
    Hashmap* jsonDefaultHeaders = (Hashmap*)JsonParse(StringFrom(
"{\
    \"Content-Type\":\"text/html\",\
    \"Connection\":\"Closed\"\
}"
        ,&globalArena),&globalArena)->ptr;
    defaultHeaders = ArenaAlloc(&globalArena,sizeof(Hashmap));
    *defaultHeaders = HashmapNew(sizeof(String),&globalArena);
    MapKeys jsonkeys = HashmapKeys(jsonDefaultHeaders);
    for(int i=0;i<jsonkeys.size;i++) {
        char* key = jsonkeys.keys[i];
        HashmapSet(defaultHeaders,key,(String*)((JsonElem*)HashmapGet(jsonDefaultHeaders,key))->ptr);
    }
}

struct Server ServerInit(int domain,int service,int protocol,u_long interface,int port,int backlog) {
    //defaultHeaders = (Hashmap*)JsonParse(StringFrom("{\"Content-Type\":\"text/html\",\"Connection\":\"Closed\"}",&globalArena),&globalArena)->ptr;
    ServerInitDefault();
    struct Server server = {0};

    server.domain = domain;
    server.service = service;
    server.protocol = protocol;
    server.interface = interface;
    server.port = port;
    server.backlog = backlog;

    server.address.sin_family = domain;
    server.address.sin_port = htons(port);
    server.address.sin_addr.s_addr = htonl(interface);

    server.socket = socket(domain,service,protocol);
    if (server.socket == 0) {
        perror("failed to connect socket :(\n");
        exit(1);
    }

    if (setsockopt(server.socket,SOL_SOCKET,SO_REUSEADDR, &(int){1},sizeof(int))<0){
        perror("failed to reuse address socket :(\n");
        exit(1);
    }

    if (setsockopt(server.socket,SOL_SOCKET,SO_REUSEPORT, &(int){1},sizeof(int))<0){
        perror("failed to reuse port socket :(\n");
        exit(1);
    }

    if (bind(
        server.socket,
        (struct sockaddr*)&server.address,
        sizeof(server.address)
    ) < 0) {
        perror("failed to bind socket :(\n");
        exit(1);
    }

    if(listen(server.socket,server.backlog) < 0) {
        perror("failed to bind socket :(\n");
        exit(1);
    }

    return server;
}

String ServerMakeHeader(int code,int contentLength,String headers,struct Arena* arena) {
    String header = StringFormat(arena,StringFrom("HTTP/1.1 %d OK\n",arena),code);
    Hashmap finalmap = HashmapNew(sizeof(String),arena);
    MapKeys defaultKeys = HashmapKeys(defaultHeaders);
    for(int i=0;i<defaultKeys.size;i++) {
        char* key = defaultKeys.keys[i];
        HashmapSet(&finalmap,key,(String*)HashmapGet(defaultHeaders,key));
    }
    Hashmap* jsonHeadermap = (Hashmap*)JsonParse(headers,arena)->ptr;
    if (jsonHeadermap) {
        MapKeys jsonkeys = HashmapKeys(jsonHeadermap);
        for(int i=0;i<jsonkeys.size;i++) {
            char* key = jsonkeys.keys[i];
            HashmapSet(&finalmap,key,(String*)((JsonElem*)HashmapGet(jsonHeadermap,key))->ptr);
        }
    }

    if (contentLength >= 0) {
        String contentLengths = StringFromInt(contentLength,arena);
        HashmapSet(&finalmap,"Content-Length",&contentLengths);
    }
    
    header = StringConcat(header,ServerMapToHeaders(&finalmap,arena),arena);

    header = StringConcat(header,StringFrom("\r\n",arena),arena);
    return header;
}

String ServerGetRequest(int clientSocket,int maxRequestSize,struct Arena* arena) {
    String buffer = StringAlloc(maxRequestSize,arena);
    int len = recv(clientSocket,buffer.text,maxRequestSize,0);
    buffer.size = len;
    FILE* fp = fopen("./log.txt","a");
    fprintf(fp,"\n[HTTP]\n");
    fprintf(fp,StringToChar(buffer,arena));
    fprintf(fp,"\n[/HTTP]");
    fclose(fp);
    return buffer;
}

struct sockaddr_in ServerGetSenderIp(int clientSocket) {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getsockname(clientSocket, (struct sockaddr*)&addr, &addr_size);
    return addr;
}

void ServerSocketWriteTo(int clientSocket,String str,struct Arena* arena) {
    write(clientSocket,StringToChar(str,arena),str.size);
}

void ServerSocketClose(int clientSocket) {
    close(clientSocket);
}

int ServerAwaitConnection(struct Server* server) {
    int address_len = sizeof(server->address);
    return accept(server->socket,(struct sockaddr*)&server->address,(socklen_t*)&address_len);
}

int ServerPathDepth(String path) {
    int depth = 0;
    struct Arena scratch = ArenaCreate(1024);
    int i=StringFind(path,StringFrom("/",&scratch),0);
    while(i>=0) {
        if(i > 0 && path.text[i-1]=='.') {
            if (i>1 && path.text[i-2]=='.')
                depth -= 1;
        }
        else
            depth ++;
        i = StringFind(path,StringFrom("/",&scratch),i+1);
    }
    ArenaDelete(&scratch);
    return depth;
}

struct Server ServerDefaultInit(int port,int backlog){
    return ServerInit(AF_INET,SOCK_STREAM,0,INADDR_ANY,port,backlog);
}

String ServerRequestMethod(String request,struct Arena* arena) {
    struct Arena scratch = ArenaCreate(1024);
    int i = StringFind(request,StringFrom(" ",&scratch),0);
    String method = StringSub(request,0,i,arena);
    ArenaDelete(&scratch);
    return method;
}

String ServerRequestPath(String request,struct Arena* arena) {
    struct Arena scratch = ArenaCreate(1024);
    String path;
    int i = StringFind(request,StringFrom(" ",&scratch),0);
    int j = StringFind(request,StringFrom(" ",&scratch),i+1);
    path = StringSub(request,i+1,j,&scratch);
    path = StringConcat(StringFrom("./site",&scratch),path,&scratch);
    if (path.text[path.size-1] == '/') {
        path = StringConcat(path,StringFrom("index.html",&scratch),&scratch);
    }
    path = StringCpy(path,arena);
    ArenaDelete(&scratch);
    return path;
}

String ServerRequestRawPath(String request,struct Arena* arena) {
    struct Arena scratch = ArenaCreate(1024);
    String path;
    int i = StringFind(request,StringFrom(" ",&scratch),0);
    int j = StringFind(request,StringFrom(" ",&scratch),i+1);
    path = StringSub(request,i+1,j,&scratch);
    path = StringCpy(path,arena);
    ArenaDelete(&scratch);
    return path;
}

String ServerFindContent(String request,struct Arena* arena) {
    struct Arena scratch = ArenaCreate(1024);
    String content;
    int i = StringFind(request,StringFrom("\n\n",&scratch),0);
    if (i == -1) {
        i = StringFind(request,StringFrom("\n\r\n",&scratch),0);
    }
    content = StringSub(request,i,request.size,arena);
    ArenaDelete(&scratch);
    return content;
}

void ServerRespond(int code,String headers,String content,struct Arena* arena) {
    String response = StringConcat(ServerMakeHeader(code,content.size,headers,arena),content,arena);
    printf("%s",StringToChar(response,arena));
}

String ServerStrip(String path,struct Arena* arena) {
    int iq = StringFind(path,StringFrom("?",arena),0);
    if (iq < 0) {
        return path;
    }
    return StringSub(path,0,iq,arena);
}

String ServerPathExtension(String path,struct Arena* arena) {
    printf("\nstripped: %s\n\n",StringToChar(ServerStrip(path,arena),arena));
    int iextension = StringFindLast(ServerStrip(path,arena),StringFrom(".",arena),0);
    String extension = StringFrom("plain",arena);
    int iend = path.size;
    int iq = StringFind(path,StringFrom("?",arena),iextension);
    printf("i: %d\n",iq);
    iend = iq != -1 ? iq : iend;
    if (iextension != -1)
        extension = StringSub(path,iextension+1,iend,arena);
    return extension;
}

String ServerRequestContent(String request,struct Arena* arena) {
    int icontent = StringFind(request,StringFrom("\n\n",arena),0);
    printf("icontent: %d %d\n",icontent,request.size);
    if (icontent < 0)
        return StringFrom("",arena);
    return StringSub(request,icontent+2,request.size,arena);
}

void ServerRun(struct Server* server) {
    if (server->onLaunch)
        server->onLaunch(server);
    while (1) {
        struct Arena sarena = ArenaCreate(1024);
        if(server->onAwait)
            server->onAwait(server);

        printf("== awaiting connection ==\n");
        int clientSocket = ServerAwaitConnection(server);
        
        struct timeval stop, start;
        gettimeofday(&start, NULL);
        /*
        struct sockaddr_in addr = ServerGetSenderIp(clientSocket);
        printf("/////// ip:%s:%d ///////\n\n",inet_ntoa(addr.sin_addr),addr.sin_port);
        */
        String request = ServerGetRequest(clientSocket,MAXREQUESTSIZE,&sarena);
        if (request.size > 1) {
        printf("%s\n\n",StringToChar(request,&sarena));

        if(server->onRequest)
            server->onRequest(server,request);

        String rawPath = ServerRequestRawPath(request,&sarena);

        String path = ServerRequestPath(request,&sarena);
        printf("path: %s\n",StringToChar(path,&sarena));
        String strippedPath = ServerStrip(path,&sarena);

        String method = ServerRequestMethod(request,&sarena);
        printf("method: %s\n",StringToChar(method,&sarena));

        String extension = ServerPathExtension(path,&sarena);
        printf("extension: %s\n",StringToChar(extension,&sarena));

        String content = ServerRequestContent(request,&sarena);
        if(content.size <= 0) {
            content = StringFrom("",&sarena);
        }

        FILE* fpl = fopen("./log.txt","a");
        fprintf(fpl,"\n[CONTENT]");
        fprintf(fpl,StringToChar(content,&sarena));
        fprintf(fpl,"[/CONTENT]");
        fclose(fpl);
        
        printf("content: %s\n",StringToChar(content,&sarena));

        String response;
        FILE* fp = fopen(StringToChar(ServerStrip(path,&sarena),&sarena),"r");
        ServerSocketWriteTo(clientSocket,StringFrom("",&sarena),&sarena);
        if (fp == NULL || ServerPathDepth(path) < 1) {
            printf("incorrect path\n");
            String content = StringFrom("<html><body> <h1>404</h1> <h2>no such file</h2> </body></html>",&sarena);
            response = StringConcat(ServerMakeHeader(404,content.size,StringFrom("{}",&sarena),&sarena),content,&sarena);
            ServerSocketWriteTo(clientSocket,response,&sarena);
        }else {
            if(StringEq(extension,StringFrom("c",&sarena))) {
                char* binpath = StringToChar(StringSub(path,0,StringFindLast(strippedPath,StringFrom(".",&sarena),0),&sarena),&sarena);
                printf("binpath: %s\n",binpath);
                FILE* fp = fopen(binpath,"r");
                if(DEBUG) {
                    if(fp) {
                        remove(binpath);
                    }
                }

                if(!fp) {
                    char* command = StringToChar(StringFormat(&sarena,StringFrom("cd /myserver && gcc %S -o %s -lm -I/usr/include/postgresql -lpq",&sarena),strippedPath,binpath),&sarena);
                    printf("command: %s\n",command);
                    FILE* pfp = popen(command,"r");
                    pclose(pfp);
                    
                    fp = fopen(binpath,"r");
                }
                
                if(fp) {
                    fclose(fp);
                    char* command = StringToChar(StringFormat(&sarena,StringFrom("cd /myserver && chmod +x %s && %s %S '%S' '%S' ",&sarena),binpath,binpath,method,rawPath,content),&sarena);
                    FILE* fpl = fopen("./log.txt","a");
                    fprintf(fpl,"\n[COMMAND]");
                    fprintf(fpl,command);
                    fprintf(fpl,"[/COMMAND]");
                    fclose(fpl);
                    printf("command: %s\n",command);
                    FILE* responsefp = popen(command,"r");
                    if (responsefp) {
                        response = StringFrom("",&sarena);
                        for(char c=getc(responsefp);!feof(responsefp);c=getc(responsefp)) 
                            response = StringConcat(response,StringFromChar(c,&sarena),&sarena);
                        pclose(responsefp);
                        printf("\n\n//response://\n%s\n\n",StringToChar(response,&sarena));
                        ServerSocketWriteTo(clientSocket,response,&sarena);   
                    }else {
                        String content = StringFrom("<html><body> <h1>500</h1> <h2>server error</h2> </body></html>",&sarena);
                        response = StringConcat(ServerMakeHeader(500,content.size,StringFrom("{}",&sarena),&sarena),content,&sarena);
                        ServerSocketWriteTo(clientSocket,response,&sarena);
                    }
                }else {
                    printf("error in file compilation\n");
                    String content = StringFrom("<html><body> <h1>500</h1> <h2>server error</h2> </body></html>",&sarena);
                    response = StringConcat(ServerMakeHeader(500,content.size,StringFrom("{}",&sarena),&sarena),content,&sarena);
                    ServerSocketWriteTo(clientSocket,response,&sarena);
                }
            }else {
                fseek(fp, 0, SEEK_END);
                long int size = ftell(fp);
                fseek(fp,0,0);
                String content = StringAlloc(size,&sarena);
                for(int i=0;i<size;i++) 
                    content.text[i] = fgetc(fp);
                String contentType = StringConcat(StringFrom("text/",&sarena),extension,&sarena);
                response = StringConcat(ServerMakeHeader(200,content.size,StringFormat(&sarena,StringFrom("{\"Content-Type\":\"%S\"}",&sarena),contentType),&sarena),content,&sarena);
                printf("\n\n//response://\n%s\n\n",StringToChar(response,&sarena));
                ServerSocketWriteTo(clientSocket,response,&sarena);
            }
        }

        if (server->onGET && StringEq(method,StringFrom("GET",&sarena)))
            server->onGET(server,path);
        
        if (server->onPOST && StringEq(method,StringFrom("POST",&sarena)))
            server->onPOST(server,path);
    }
        gettimeofday(&stop, NULL);
        unsigned long dt = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
        FILE* fpl = fopen("./log.txt","a");
        fprintf(fpl,"\n[TIME]");
        fprintf(fpl,StringToChar(StringFromLong((long)dt,&sarena),&sarena));
        fprintf(fpl," us[/TIME]");
        fclose(fpl);
        ServerSocketClose(clientSocket);
        ArenaDelete(&sarena);
    }
}


Hashmap ServerParseGET(char* rawurl,struct Arena* arena) {
    Hashmap map = HashmapNew(sizeof(String),arena);
    String url = StringFrom(rawurl,arena);
    int start = StringFind(url,StringFrom("?",arena),0);
    if(start == -1) {
        return map;
    }
    int i = StringFind(url,StringFrom("&",arena),start+1);
    int j = start+1;
    while(i != -1) {
        int k = StringFind(url,StringFrom("=",arena),j);
        String key = StringSub(url,j,k,arena);
        String* val = ArenaAlloc(arena,sizeof(String));
        *val = StringSub(url,k+1,i,arena);
        HashmapSet(&map,StringToChar(key,arena),val);
        j = i+1;
        i = StringFind(url,StringFrom("&",arena),i+1);;
    }
    int k = StringFind(url,StringFrom("=",arena),j);
    if (k < 0)
        return map;
    String key = StringSub(url,j,k,arena);
    String* val = ArenaAlloc(arena,sizeof(String));
    *val = StringSub(url,k+1,url.size,arena);
    HashmapSet(&map,StringToChar(key,arena),val);
    return map;
}

Hashmap ServerParsePOST(char* rawcontent,struct Arena* arena) {
    Hashmap map = HashmapNew(sizeof(String),arena);
    String content = StringFrom(rawcontent,arena);
    int start = 0;
    int i = StringFind(content,StringFrom("&",arena),start);
    int j = start;
    while(i != -1) {
        int k = StringFind(content,StringFrom("=",arena),j);
        String key = StringSub(content,j,k,arena);
        String* val = ArenaAlloc(arena,sizeof(String));
        *val = StringSub(content,k+1,i,arena);
        HashmapSet(&map,StringToChar(key,arena),val);
        j = i+1;
        i = StringFind(content,StringFrom("&",arena),i+1);;
    }
    int k = StringFind(content,StringFrom("=",arena),j);
    if (k < 0)
        return map;
    String key = StringSub(content,j,k,arena);
    String* val = ArenaAlloc(arena,sizeof(String));
    *val = StringSub(content,k+1,content.size,arena);
    HashmapSet(&map,StringToChar(key,arena),val);
    return map;
}

Hashmap ServerParseRequest(char** argv,struct Arena* arena) {
    String method = StringFrom(argv[1],arena);
    if(StringEq(method,StringFrom("POST",arena)))
        return ServerParsePOST(argv[3],arena);
    return ServerParseGET(argv[2],arena);
}