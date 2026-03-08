#ifndef MYJSON

#define MYJSON
#include "json.h"

struct Arena JsonInitArena() {
    JSON_ARENA = ArenaCreate(1024);
    return JSON_ARENA;
}

String JsonFromList(List* list,struct Arena* arena) {
    struct Arena scratch = ArenaCreate(1024);
    String str = StringFrom("[",&scratch);
    ListNode* curr = &list->head;
    for(int i=0;i<list->size;i++) {
        ListValue val = curr->val;
        JsonElem elem = *(JsonElem*)val.ptr;
        switch (elem.type) {
            case INT:{
                int n = *(int*)elem.ptr;
                str = StringFormat(&scratch,StringFrom("%S %d",&scratch),str,n);
                break;
            }
            case BOOL:{
                int b = *(int*)elem.ptr;
                String boolstr = b?StringFrom(" true",&scratch):StringFrom(" false",&scratch);
                str = StringConcat(str,boolstr,&scratch);
                break;
            }
            case FLOAT:{
                float f = *(float*)elem.ptr;
                str = StringFormat(&scratch,StringFrom("%S %f",&scratch),str,f);
                break;
            }
            case STRING:{
                String s = *(String*)elem.ptr;
                str = StringFormat(&scratch,StringFrom("%S \"%S\"",&scratch),str,s);
                break;
            }
            case LIST:{
                List* list = (List*)elem.ptr;
                str = StringFormat(&scratch,StringFrom("%S %S",&scratch),str,JsonFromList(list,&scratch));
                break;
            }
            case OBJECT:{
                Hashmap* map = (Hashmap*)elem.ptr;
                str = StringFormat(&scratch,StringFrom("%S %S",&scratch),str,JsonFromHashmap(map,&scratch));
                break;
            }
        }
        if(i!=list->size-1) {
            str = StringConcat(str,StringFrom(",",&scratch),&scratch);
        }
        curr = curr->next;
    }
    str = StringConcat(str,StringFrom(" ]",&scratch),&scratch);
    str = StringCpy(str,arena);
    ArenaDelete(&scratch);
    return str;
}

String jfromlist(List* list) {
    return JsonFromList(list,&JSON_ARENA);
}

String JsonFromHashmap(Hashmap* map,struct Arena *arena) {
    struct Arena scratch = ArenaCreate(1024);
    String str = StringFrom("{",&scratch);
    MapKeys keys = HashmapKeys(map);
    for(int i=0;i<keys.size;i++) {
        char* key = keys.keys[i];
        str = StringFormat(&scratch,StringFrom("%S \"%s\":",&scratch),str,key);
        JsonElem* elem = HashmapGet(map,key);
        switch (elem->type) {
            case INT:{
                int n = *(int*)elem->ptr;
                str = StringConcat(str,StringFromInt(n,&scratch),&scratch);
                //str = StringFormat(&scratch,StringFrom("%S %d",&scratch),str,n);
                break;
            }
            case BOOL:{
                int b = *(int*)elem->ptr;
                String boolstr = b?StringFrom(" true",&scratch):StringFrom(" false",&scratch);
                str = StringConcat(str,boolstr,&scratch);
                break;
            }
            case FLOAT:{
                float f = *(float*)elem->ptr;
                str = StringFormat(&scratch,StringFrom("%S %f",&scratch),str,f);
                break;
            }
            case STRING:{
                String s = *(String*)elem->ptr;
                str = StringFormat(&scratch,StringFrom("%S \"%S\"",&scratch),str,s);
                break;
            }
            case LIST:{
                List* list = (List*)elem->ptr;
                str = StringFormat(&scratch,StringFrom("%S %S",&scratch),str,JsonFromList(list,&scratch));
                break;
            }
            case OBJECT:{
                Hashmap* map = (Hashmap*)elem->ptr;
                str = StringFormat(&scratch,StringFrom("%S %S",&scratch),str,JsonFromHashmap(map,&scratch));
                break;
            }
        }
        if(i!=keys.size-1) {
            str = StringConcat(str,StringFrom(",",&scratch),&scratch);
        }
    }
    str = StringConcat(str,StringFrom("}",&scratch),&scratch);
    str = StringCpy(str,arena);
    ArenaDelete(&scratch);
    return str;
}

String jfrommap(Hashmap* map) {
    return JsonFromHashmap(map,&JSON_ARENA);
}

String JsonToString(JsonElem* elem,struct Arena* arena) {
    if( elem->type == OBJECT)
        return JsonFromHashmap(elem->ptr,arena);
    else if (elem->type == LIST)
        return JsonFromList(elem->ptr,arena);
    return StringFrom("",arena);
}

String jtostring(JsonElem* elem) {
    return JsonToString(elem,&JSON_ARENA);
}

JsonElem* JsonParseElem(String str,int start,int*icurrent,Hashmap* map,struct Arena* arena) {
    struct Arena scratch = ArenaCreate(1024);
    int icomma = StringFindCharEscape(str,',',start);
    int ilistend = StringFindCharEscape(str,']',start);
    int iobjectend = StringFindCharEscape(str,'}',start);
    icomma = (icomma != -1) && ((ilistend == -1) || icomma < ilistend) ? icomma : ilistend;
    icomma = (icomma != -1) && ((iobjectend == -1 )|| icomma < iobjectend) ? icomma : iobjectend;

    int iliststart = StringFindCharEscape(str,'[',start);
    int iobjectstart = StringFindCharEscape(str,'{',start);
    int iquote = StringFindCharEscape(str,'"',start);
    int indices[] = {icomma,iliststart,iobjectstart,iquote};
    int mini = 0;
    int minj = indices[0];
    for(int i=0;i<4;i++) {
        int j = indices[i];
        if(j >= 0 && j < minj) {
            mini = i;
            minj = j;
        }
    }
    *icurrent = icomma;
    switch(mini) {
        // ,
        case 0:{
            char nums[] = {'0','1','2','3','4','5','6','7','8','9'};
            char c;
            int istart = -1;
            int iend = -1;
            int isnumber = 1;
            int isreal = 0;
            for(int i=start;i<icomma;i++) {
                c = str.text[i];
                if(c == '.')
                    isreal = 1;
                if (istart >= 0) {
                    
                    if (c == ' ') {
                        break;
                    }
                    iend++;
                    isnumber = 0;
                    for(int j=0;j<10;j++) {
                        if (c == nums[j] || c == '.') {
                            isnumber = 1;
                            break;
                        }
                    }
                    if(!isnumber) break;
                }else {
                    for(int j=0;j<10;j++) {
                        if(c == nums[j]) {
                            if(istart == -1){
                                istart = i;
                                iend = i+1;
                                break;
                            }
                            iend++;
                        }
                    }
                }
            }
            if (isnumber) {
                if (isreal) {
                    float* n = ArenaAlloc(arena,sizeof(float));
                    int err;
                    *n = StringToFloat(StringSub(str,istart,iend,&scratch),&err);
                    if(err) {
                        printf("<JsonParseElem> error in string to float\n");
                        ArenaDelete(&scratch);
                        return NULL;
                    }
                    JsonElem* elem = ArenaAlloc(arena,sizeof(JsonElem));
                    elem->ptr = n;
                    elem->type = FLOAT;
                    ArenaDelete(&scratch);
                    return elem;
                }else {
                    int* n = ArenaAlloc(arena,sizeof(int));
                    int err;
                    *n = StringToInt(StringSub(str,istart,iend,&scratch),&err);
                    if(err) {
                        printf("<JsonParseElem> error in string to int\n");
                        ArenaDelete(&scratch);
                        return NULL;
                    }
                    JsonElem* elem = ArenaAlloc(arena,sizeof(JsonElem));
                    elem->ptr = n;
                    elem->type = INT;
                    ArenaDelete(&scratch);
                    return elem;
                }
            }else {
                char *C;
                int i = start;
                for(C = &str.text[start];*C == ' ';C++){i++;};
                if (C && *C == 't' || *C == 'f') {
                    int itrue = StringFind(str,StringFrom("true",&scratch),start);
                    int ifalse = StringFind(str,StringFrom("false",&scratch),start);
                    if (itrue == i || ifalse == i) {
                        JsonElem* elem = ArenaAlloc(arena,sizeof(JsonElem));
                        int* b = ArenaAlloc(arena,sizeof(int));
                        *b = (itrue == i) || (ifalse != i);
                        elem->ptr = b;
                        elem->type = BOOL;
                        ArenaDelete(&scratch);
                        return elem;
                    }
                }
            }
            ArenaDelete(&scratch);
            return NULL;
            break;
        }
        // [
        case 1:{
            List* list = ArenaAlloc(arena,sizeof(List));
            *list = ListNew(arena);
            int i = minj+1;
            while (*icurrent < ilistend) {
                JsonElem* elem = JsonParseElem(str,i,icurrent,map,arena);
                i = *icurrent+1;
                if(!elem) 
                    return NULL;
                ListAppendJsonElem(list,elem);
            }
            JsonElem* elem = ArenaAlloc(arena,sizeof(JsonElem));
            elem->ptr = list;
            elem->type = LIST;
            ArenaDelete(&scratch);
            return elem;
            break;
        }
        // {
        case 2:{
            Hashmap* m = JsonParseObject(str,arena,minj,icurrent);
            JsonElem* elem = ArenaAlloc(arena,sizeof(JsonElem));
            elem->ptr = m;
            elem->type = OBJECT;
            ArenaDelete(&scratch);
            return elem;
            break;
        }
        // "
        case 3:{
            int* index = StringFindString(str,minj,&scratch);
            if(index) {
                String* s = ArenaAlloc(arena,sizeof(String));
                *s = StringSub(str,index[0],index[1],arena);
                JsonElem* elem = ArenaAlloc(arena,sizeof(JsonElem));
                elem->ptr = s;
                elem->type = STRING;
                ArenaDelete(&scratch);
                return elem;
            }else
                return NULL;
            break;
        }
    }
    return NULL;
}

Hashmap* JsonParseObject(String str,struct Arena* arena,int start,int* icurrent) {
    Hashmap* map = ArenaAlloc(arena,sizeof(Hashmap));
    *map = HashmapNew(sizeof(JsonElem),arena);
    struct Arena scratch = ArenaCreate(1024);

    *icurrent = StringFind(str,StringFrom("{",&scratch),start);
    int ok = *icurrent >= 0;
    while (ok) {
        int* stri = StringFindString(str,*icurrent+1,&scratch);
        if(!stri) return NULL;

        String key = StringSub(str,stri[0],stri[1],&scratch);
        int i = StringFind(str,StringFrom(":",&scratch),stri[1]+1);
        JsonElem* elem = JsonParseElem(str,i+1,icurrent,map,arena);
        if(!elem) return NULL;
        HashmapSet(map,StringToChar(key,arena),elem);

        ok = str.text[*icurrent] != '}';
    }
    (*icurrent)++;
    ArenaDelete(&scratch);
    return map;
}

JsonElem* JsonParse(String str,struct Arena* arena) {
    Hashmap* map = ArenaAlloc(arena,sizeof(Hashmap));
    *map = HashmapNew(sizeof(JsonElem),arena);
    int icurrent=0;
    return JsonParseElem(str,0,&icurrent,map,arena);
}

JsonElem* jparse(String str) {
    return JsonParse(str,&JSON_ARENA);
}

void HashmapSetInt(Hashmap* map,char* key,int n) {
    int* ptr = ArenaAlloc(map->arena,sizeof(int));
    *ptr = n;
    JsonElem* elem = ArenaAlloc(map->arena,sizeof(JsonElem));
    *elem = (JsonElem){.ptr=ptr,.type=INT};
    HashmapSet(map,key,elem);
}
void HashmapSetBool(Hashmap* map,char* key,int b) {
    int* ptr = ArenaAlloc(map->arena,sizeof(int));
    *ptr = b;
    JsonElem* elem = ArenaAlloc(map->arena,sizeof(JsonElem));
    *elem = (JsonElem){.ptr=ptr,.type=BOOL};
    HashmapSet(map,key,elem);
}
void HashmapSetFloat(Hashmap* map,char* key,float n) {
    float* ptr = ArenaAlloc(map->arena,sizeof(float));
    *ptr = n;
    JsonElem* elem = ArenaAlloc(map->arena,sizeof(JsonElem));
    *elem = (JsonElem){.ptr=ptr,.type=FLOAT};
    HashmapSet(map,key,elem);
}
void HashmapSetString(Hashmap* map,char* key,String s) {
    String* ptr = ArenaAlloc(map->arena,sizeof(String));
    *ptr = s;
    JsonElem* elem = ArenaAlloc(map->arena,sizeof(JsonElem));
    *elem = (JsonElem){.ptr=ptr,.type=STRING};
    HashmapSet(map,key,elem);
}
void HashmapSetList(Hashmap* map,char* key,List list) {
    List* ptr = ArenaAlloc(map->arena,sizeof(List));
    *ptr = list;
    JsonElem* elem = ArenaAlloc(map->arena,sizeof(JsonElem));
    *elem = (JsonElem){.ptr=ptr,.type=LIST};
    HashmapSet(map,key,elem);
}
void HashmapSetMap(Hashmap* map,char* key,Hashmap m) {
    Hashmap* ptr = ArenaAlloc(map->arena,sizeof(Hashmap));
    *ptr = m;
    JsonElem* elem = ArenaAlloc(map->arena,sizeof(JsonElem));
    *elem = (JsonElem){.ptr=ptr,.type=OBJECT};
    HashmapSet(map,key,elem);
}

void ListAppendJsonElem(List* list,JsonElem* elem) {
    ListValue val = {.ptr=elem};
    ListAppendVal(list,val);
}

void ListAppendInt(List* list,int n) {
    int* ptr = ArenaAlloc(list->arena,sizeof(int));
    *ptr = n;
    JsonElem* elem = ArenaAlloc(list->arena,sizeof(JsonElem));
    *elem = (JsonElem){.ptr=ptr,.type=INT};
    ListValue val = {.ptr=elem};
    ListAppendVal(list,val);
}

void ListSetInt(List* list,int index,int n) {
    int* ptr = ArenaAlloc(list->arena,sizeof(int));
    *ptr = n;
    JsonElem* elem = ArenaAlloc(list->arena,sizeof(JsonElem));
    *elem = (JsonElem){.ptr=ptr,.type=INT};
    ListValue val = {.ptr=elem};
    ListSetVal(list,index,val);
}

void ListInsertInt(List* list,int index,int n) {
    int* ptr = ArenaAlloc(list->arena,sizeof(int));
    *ptr = n;
    JsonElem* elem = ArenaAlloc(list->arena,sizeof(JsonElem));
    *elem = (JsonElem){.ptr=ptr,.type=INT};
    ListValue val = {.ptr=elem};
    ListInsertVal(list,index,val);
}


void ListAppendFloat(List* list,float n) {
    float* ptr = ArenaAlloc(list->arena,sizeof(float));
    *ptr = n;
    JsonElem* elem = ArenaAlloc(list->arena,sizeof(JsonElem));
    *elem = (JsonElem){.ptr=ptr,.type=FLOAT};
    ListValue val = {.ptr=elem};
    ListAppendVal(list,val);
}

void ListSetFloat(List* list,int index,float n) {
    float* ptr = ArenaAlloc(list->arena,sizeof(float));
    *ptr = n;
    JsonElem* elem = ArenaAlloc(list->arena,sizeof(JsonElem));
    *elem = (JsonElem){.ptr=ptr,.type=FLOAT};
    ListValue val = {.ptr=elem};
    ListSetVal(list,index,val);
}

void ListInsertFloat(List* list,int index,float n) {
    float* ptr = ArenaAlloc(list->arena,sizeof(float));
    *ptr = n;
    JsonElem* elem = ArenaAlloc(list->arena,sizeof(JsonElem));
    *elem = (JsonElem){.ptr=ptr,.type=FLOAT};
    ListValue val = {.ptr=elem};
    ListInsertVal(list,index,val);
}

/*
int main() {
    struct Arena sarena = StringInitArena();
    struct Arena arena = ArenaCreate(1024);
    struct Arena jarena = JsonInitArena();
    
    Hashmap map = HashmapNew(sizeof(JsonElem),&arena);

    HashmapSetInt(&map,"int",-100000000);

    HashmapSetBool(&map,"bool",1);

    HashmapSetFloat(&map,"float",3.14);

    HashmapSetString(&map,"string",StringFrom("test yay yay",&arena));

    List list = ListNew(&arena);
    for(int i=0;i<10;i++) {
        ListAppendInt(&list,10000*i);
    }
    HashmapSetList(&map,"list",list);

    String mapstr = JsonFromHashmap(&map,&arena);
    printf("\ngenerated json: %s\n\n",stochar(mapstr));
    */
    /*
    String strTest = StringFrom("    \"key\"   ",&sarena);
    int* stri = StringFindString(strTest,0,&sarena);
    if(stri) {
        String subtest = StringSub(strTest,stri[0],stri[1],&sarena);
        printf("%s\n",stochar(subtest));
    }
    */
    /*
    String rawjson = Str("[\n5,\n{\n\"key\":400,\n\"test\": {\"test\": \"yay\"} ,\n   \"jsp moi\" :  13.67},\n \",,,,\" \n]");
    JsonElem* json = jparse(rawjson);
    if(json) {
        printf("raw json: %s\n",stochar(rawjson));
        printf("parsed json: %s\n\n",stochar(jtostring(json)));
    }else {
        printf("error in json\n\n");
    }
    ArenaDelete(&arena);
    ArenaDelete(&sarena);
    ArenaDelete(&jarena);
}*/

#endif