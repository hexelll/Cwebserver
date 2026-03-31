#include <stdio.h>
#include "/myserver/libs/server.c"

int main(int argc,char** argv) {
    ServerInitDefault();
    struct Arena sarena = ArenaCreate(1024);

    ServerRespond(200,StringFrom("{\"Content-Type\":\"application/json\"}",&sarena),StringFrom("{\"param\":[{\"test\":\"feur~éà$`\"}]}",&sarena),&sarena);
    ArenaDelete(&sarena);
}