#include "hashmap.c"
#include "string.c"
#include "list.c"

struct Arena JSON_ARENA = {0};

typedef enum {
    INT,
    BOOL,
    FLOAT,
    STRING,
    LIST,
    OBJECT,
}jsonType;

typedef struct {
    void* ptr;
    jsonType type;
}JsonElem;

void HashmapSetInt(Hashmap* map,char* key,int n);
void HashmapSetBool(Hashmap* map,char* key,int b);
void HashmapSetFloat(Hashmap* map,char* key,float n);
void HashmapSetString(Hashmap* map,char* key,String s);
void HashmapSetList(Hashmap* map,char* key,List list);
void HashmapSetMap(Hashmap* map,char* key,Hashmap m);

void ListAppendJsonElem(List* list,JsonElem* elem);
void ListAppendInt(List* list,int n);
void ListSetInt(List* list,int index,int n);
void ListInsertInt(List* list,int index,int n);

void ListAppendFloat(List* list,float n);
void ListSetFloat(List* list,int index,float n);
void ListInsertFloat(List* list,int index,float n);

Hashmap* JsonParseObject(String str,struct Arena* arena,int start,int* icurrent);
JsonElem* JsonParseElem(String str,int start,int*icurrent,Hashmap* map,struct Arena* arena);
JsonElem* JsonParse(String str,struct Arena* arena);
JsonElem* jparse(String str);

String JsonFromList(List* list,struct Arena* arena);
String jfromlist(List* list);
String JsonFromHashmap(Hashmap* map,struct Arena *arena);
String jfrommap(Hashmap* map);
String JsonToString(JsonElem* elem,struct Arena* arena);
String jtostring(JsonElem* elem);

struct Arena JsonInitArena();