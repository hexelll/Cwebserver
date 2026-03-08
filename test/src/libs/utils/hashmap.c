#ifndef MYHASHMAP
#define MYHASHMAP
#include <stdlib.h>
#include <string.h>
#include "hashmap.h"
#define MAP_SIZE 100
#define ARENA_SIZE 10

unsigned long hash(char* str){
    unsigned long hash = 5381;
    for(int c = *str, i=0; c!='\0' ; c=*(str+i),i++)
        hash = ((hash << 5) + hash) + c; //hash*33+c = hash*(32+1)+c = hash*32 + hash + c = hash*2^5 + hash + c = hash << 5 + hash + c
    return hash;
}

int stringeq(char* a,char* b) {
    if (!a || !b) {
        return 0;
    }
    char c1 = *a;
    char c2 = *b;
    int ok = c1 == c2;
    while(c1 == c2 && c1 != '\0' && c2 != '\0') {
        a++;
        b++;
        c1 = *a;
        c2 = *b;
        ok = c1 == c2;
    }
    return ok;
}

Hashmap HashmapNew(unsigned long itemsize,struct Arena* arena) {
    Hashmap map = {
        .buckets = NULL,
        .collection = NULL,
        .itemsize = itemsize,
        .size = MAP_SIZE,
        .collectionfill = 0,
        .arena = arena,
    };
    map.buckets = (MapElem*)ArenaAlloc(arena,sizeof(MapElem)*map.size);
    map.collection = ArenaAlloc(arena,itemsize*map.size);
    for(int i=0;i<map.size;i++) {
        map.buckets[i] = (MapElem){.index=-1,.key=NULL};
    }
    return map;
}

void HashmapResize(Hashmap* map,int newsize) {
    MapElem* newbuckets = (MapElem*)ArenaAlloc(map->arena,sizeof(MapElem)*newsize);
    for(int i=0;i<newsize;i++) {
        newbuckets[i].index = -1;
    }
    for(int i=0;i<map->size;i++) {
        MapElem elem = map->buckets[i];
        if (elem.index >= 0) {
            unsigned long hashi = hash(elem.key) % newsize;
            int set = 0;
            for(int j=hashi;!set;j=(j+1)%newsize) {
                MapElem e = newbuckets[j];
                if (e.index < 0) {
                    newbuckets[j].key = elem.key;
                    newbuckets[j].index = elem.index;
                    set = 1;
                }
            }
        }
    }
    void* newcollection = ArenaAlloc(map->arena,map->itemsize*newsize);
    memcpy(newcollection,map->collection,map->size);
    map->collection = newcollection;
    map->buckets = newbuckets;
    map->size = newsize;
}

int HashmapSet(Hashmap* map,char* key,void* data) {
    unsigned long hashi = hash(key) % map->size;
    int set = 0;
    int collision = 0;
    int n = 0;
    for(int i=hashi;!set;i=(i+1)%map->size) {
        n++;
        if (n >= map->size) {
            HashmapResize(map,map->size*2);
            n = 0;
        }
        MapElem elem = map->buckets[i];
        if (elem.index == -1) {
            map->buckets[i].key = key;
            int index = map->collectionfill++;
            map->buckets[i].index = index;
            memcpy(map->collection + index*(map->itemsize), data, map->itemsize);
            set = 1;
        }else if(stringeq(elem.key,key)) {
            memcpy(map->collection + elem.index*(map->itemsize), data, map->itemsize);
            set = 1;
        }else
            collision = 1;
    }
    return collision;
}

void* HashmapGet(Hashmap* map,char* key) {
    unsigned long hashi = hash(key) % map->size;
    int get = 0;
    int collision = 0;
    int n = 0;
    void* data = ArenaAlloc(map->arena,map->itemsize);
    for(int i=hashi;!get;i=(i+1)%map->size) {
        n++;
        if (n >= map->size) {
            return NULL;
        }
        MapElem elem = map->buckets[i];
        if (stringeq(elem.key,key)) {
            memcpy(data,map->collection+elem.index*map->itemsize,map->itemsize);
            get = 1;
        }
    }
    return data;
}

void HashmapShrinkCollection(Hashmap* map) {
    void* newcollection = ArenaAlloc(map->arena,map->size*map->itemsize);
    int n = 0;
    for(int i=0;i<map->size;i++) {
        MapElem elem = map->buckets[i];
        if (elem.index >= 0) {
            memcpy(newcollection+n*map->itemsize,map->collection+elem.index*map->itemsize,map->itemsize);
            map->buckets[i].index = n++;
        }
    }
    map->collectionfill = n;
    map->collection = newcollection;
}

void* HashmapRemove(Hashmap* map,char* key) {
    unsigned long hashi = hash(key) % map->size;
    int get = 0;
    int collision = 0;
    void* data = ArenaAlloc(map->arena,map->itemsize);
    for(int i=hashi;!get;i=(i+1)%map->size) {
        MapElem elem = map->buckets[i];
        if (stringeq(elem.key,key)) {
            memcpy(data,map->collection+elem.index*map->itemsize,map->itemsize);
            map->buckets[i].index = -2;
            map->buckets[i].key = NULL;
            get = 1;
        }
    }
    HashmapShrinkCollection(map);
    return data;
}

MapKeys HashmapKeys(Hashmap* map) {
    char** keys = ArenaAlloc(map->arena,sizeof(char*)*map->size);
    int size = 0;
    for(int i=0;i<map->size;i++) {
        MapElem elem = map->buckets[i];
        if(elem.index >= 0) {
            keys[size++] = elem.key;
        }
    }
    return (MapKeys){.keys=keys,.size=size};
}

MapValues HashmapValues(Hashmap* map) {
    MapKeys keys = HashmapKeys(map);
    void* values = ArenaAlloc(map->arena,sizeof(map->itemsize)*keys.size);
    for(int i=0;i<keys.size;i++) {
        void* v = HashmapGet(map,keys.keys[i]);
        memcpy(values+i*map->itemsize,&v,map->itemsize);
    }
    return (MapValues){.values=values,.size=keys.size};
}
/*
int main() {
    struct Arena arena = ArenaCreate(ARENA_SIZE*MAP_SIZE*sizeof(int*));
    Hashmap map = HashmapNew(sizeof(int*),&arena);
    int t1 = 13;
    int t2 = 1500;
    int t3 = 345;
    HashmapSet(&map,"test",&t2);
    HashmapSet(&map,"test2",&t1);
    HashmapSet(&map,"testtest",&t3);
    HashmapRemove(&map,"test");
    int* n = (int*)HashmapGet(&map,"test");
    if (n)
        printf("%d\n",*n);
    printf("%d\n",*(int*)HashmapGet(&map,"test2"));
    printf("%d\n",*(int*)HashmapGet(&map,"testtest"));
    MapKeys keys = HashmapKeys(&map);
    for(int i = 0;i<keys.size;i++) {
        printf("key:%s\n",keys.keys[i]);
    }
    MapValues values = HashmapValues(&map);
    int** list = (int**)values.values;
    for(int i = 0;i<values.size;i++) {
        printf("value:%d\n",*list[i]);
    }
    ArenaDelete(&arena);
}*/
#endif