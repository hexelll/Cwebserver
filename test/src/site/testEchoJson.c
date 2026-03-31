#include <stdio.h>
#include "/myserver/libs/server.c"

int main(int argc,char** argv) {
    ServerInitDefault();
    struct Arena sarena = ArenaCreate(1024);

    Hashmap map = ServerParseRequest(argv,&sarena);
    
    text = StringConcat(text,StringFrom("</ul>",&sarena),&sarena);
    ServerRespond(200,StringFrom("{\"Content-Type\":\"application/json\"}",&sarena),text,&sarena);
    ArenaDelete(&sarena);
}