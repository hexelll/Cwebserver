#include <stdio.h>
#include "/myserver/libs/server.c"

int main(int argc,char** argv) {
    struct Arena sarena = ArenaCreate(1024);

    Hashmap map = ServerParseRequest(argv,&sarena);
    String text = StringFormat(&sarena,StringFrom("url: <a href=%s>%s</a> <br> arguments: <ul>",&sarena),argv[2],argv[2]);
    MapKeys keys = HashmapKeys(&map);
    for(int i=0;i<keys.size;i++) {
        char* key = keys.keys[i];
        String value = *(String*)HashmapGet(&map,key);
        text = StringConcat(text,StringFormat(&sarena,StringFrom("\n<li>%s=%S</li>",&sarena),key,value),&sarena);
    }
    text = StringConcat(text,StringFrom("</ul>",&sarena),&sarena);
    ServerRespond(200,StringFrom("text/html",&sarena),text,&sarena);
    ArenaDelete(&sarena);
}