#include "arena.c"

typedef struct {
    int index;
    char* key;
}MapElem;

typedef struct {
    char** keys;
    int size;
}MapKeys;

typedef struct {
    void* values;
    int size;
}MapValues;

typedef struct {
    MapElem* buckets; //store the index for an item inside collection, -1 if it has not be set, -2 if it's deactivated
    void* collection; //store the item data
    unsigned long itemsize; //size of items to index into collection
    int size; // size of the buckets and the collection
    int collectionfill;
    struct Arena* arena;
}Hashmap;

unsigned long hash(char* str);

int stringeq(char* a,char* b);

Hashmap HashmapNew(unsigned long itemsize,struct Arena* arena);

void HashmapResize(Hashmap* map,int newsize);

int HashmapSet(Hashmap* map,char* key,void* data);

void* HashmapGet(Hashmap* map,char* key);

void HashmapShrinkCollection(Hashmap* map);

void* HashmapRemove(Hashmap* map,char* key);

MapKeys HashmapKeys(Hashmap* map);

MapValues HashmapValues(Hashmap* map);