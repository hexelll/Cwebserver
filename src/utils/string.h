#include "arena.c"

/*
represents a chain of characters
*/
typedef struct {
    char* text;
    int size;
}String;

/*
represents an array of String
*/
typedef struct{
    String* ptr;
    int size;
}StringArr;

/*
represents a part of a String, mostly unused for now but should be used more as it saves on memory
*/
typedef struct {
    char* text;
    int start;
    int end;
}Slice;

/*
creates the default String arena
*/
struct Arena StringInitArena();

/*
creates a String from a char*
*/
String StringFrom(char* str,struct Arena* arena);

/*
creates a String from a char* using the default String arena, you should do StringInitArena() at some point before using this function
*/
String Str(char*);

char* StringToChar(String string,struct Arena* arena);

char* stochar(String string);

String StringConcat(String a,String b,struct Arena* arena);

String sconcat(String a,String b);

String StringNew(char* text,int size);

Slice StringSlice(String s,int start,int end);

Slice sslice(String s);

String StringFromSlice(Slice s,struct Arena* arena);

String sfromslice(Slice s);

int StringToInt(String str,int* n);

float StringToFloat(String s,int* err);

String StringFromInt(int n,struct Arena* arena);

String sfromint(int n);

String StringFromFloat(float n,int afterPoint,struct Arena* arena);

String sfromfloat(float n,int afterPoint);

int StringFind(String s,String t,int start);

StringArr StringSplit(String s,String on,int start,struct Arena* arena);

StringArr ssplit(String s,String on,int start);

String StringMerge(StringArr arr,String on,struct Arena* arena);

String smerge(StringArr arr,String on);

String StringFormat(struct Arena*, String format,...);

String sformat(String format,...);

int StringEq(String a,String b);