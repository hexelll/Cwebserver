#ifndef MYSTRING
#define MYSTRING

#include "string.h"

#include <math.h>
#include <stdarg.h>

struct Arena STRING_ARENA = {0};

struct Arena StringInitArena() {
    STRING_ARENA = ArenaCreate(1024);
    return STRING_ARENA;
}

String StringFrom(char* str,struct Arena* arena) {
    char* c = str;
    int len = 0;
    while(*c != '\0') {
        len++;
        c+=sizeof(char);
    }
    char* text = ArenaAlloc(arena,sizeof(char)*len);
    for (int i=0;i<len;i++) {
        text[i] = str[i];
    }
    return StringNew(text,len);
}

String StringFromChar(char c,struct Arena* arena) {
    char* text = ArenaAlloc(arena,sizeof(char));
    *text = c;
    return StringNew(text,1);
}

String Str(char* str) {
    return StringFrom(str,&STRING_ARENA);
}


String StringAlloc(int size,struct Arena* arena) {
    char* text = ArenaAlloc(arena,size*sizeof(char));
    return StringNew(text,size);
}

String salloc(int size) {
    return StringAlloc(size,&STRING_ARENA);
}

char* StringToChar(String string,struct Arena* arena) {
    char* str = ArenaAlloc(arena,string.size+1);
    for(int i=0;i<string.size;i++) {
        str[i] = string.text[i];
    }
    str[string.size] = '\0';
    return str;
}

char* stochar(String string) {
    return StringToChar(string,&STRING_ARENA);
}

String StringConcat(String a,String b,struct Arena* arena) {
    char* text = ArenaAlloc(arena,sizeof(char)*(a.size+b.size));
    String string = StringNew(text,a.size+b.size);
    for (int i=0;i<a.size;i++)
        text[i] = a.text[i];
    for (int i=0;i<b.size;i++)
        text[i+a.size] = b.text[i];
    return string;
}

String sconcat(String a,String b) {
    return StringConcat(a,b,&STRING_ARENA);
}

String StringNew(char* text,int size) {
    return (String){
        .text=text,
        .size=size
    };
}

Slice StringSlice(String s,int start,int end) {
    return (Slice){
        .text=s.text+start*sizeof(char),
        .start=start,
        .end=end
    };
}

String StringFromSlice(Slice s,struct Arena* arena) {
    return StringNew(s.text,s.end-s.start);
}

String sfromslice(Slice s) {
    return StringFromSlice(s,&STRING_ARENA);
}

String StringSub(String s,int start,int end,struct Arena* arena) {
    Slice slice = StringSlice(s,start,end);
    return StringFromSlice(slice,arena);
}

String ssub(String s,int start,int end) {
    return StringSub(s,start,end,&STRING_ARENA);
}

int StringToInt(String str,int* err) {
    int n = 0;
    int offset = str.text[0] == '-' ? 1 : 0;
    char* nums = "0123456789";
    for(int i=offset;i<str.size;i++) {
        int found = 0;
        int ni = 0;
        for(int j=0;j<10;j++) {
            if (str.text[i]==nums[j]) {
                found = 1;
                ni=j;
                break;
            }
        }
        if (found) {
            int b = 1;
            for (int j=0;j<str.size-(i)-1;j++)
                b*=10;
            n += b*ni;
        }else {
            *err = 1;
            return 0;
        }
    }
    n *= offset==1?-1:1;
    *err = 0;
    return n;
}

long StringToLong(String str,int* err) {
    long n = 0;
    int offset = str.text[0] == '-' ? 1 : 0;
    char* nums = "0123456789";
    for(int i=offset;i<str.size;i++) {
        int found = 0;
        int ni = 0;
        for(int j=0;j<10;j++) {
            if (str.text[i]==nums[j]) {
                found = 1;
                ni=j;
                break;
            }
        }
        if (found) {
            long b = 1;
            for (int j=0;j<str.size-(i)-1;j++)
                b*=10;
            n += b*ni;
        }else {
            *err = 1;
            return 0;
        }
    }
    n *= offset==1?-1:1;
    *err = 0;
    return n;
}

float StringToFloat(String str,int* err) {
    char* nums = "0123456789";
    float n = 0;
    int indecimal = 0;
    int idec = 0;
    for(int i=0;i<str.size;i++) {
        if (str.text[i] == '.') {
            idec = i;
            break;
        }
    }
    int offset=0;
    if(str.text[0]=='-') {
        offset=1;
    }
    for (int i=offset;i<str.size;i++) {
        if (i == idec) {
            indecimal = 1;
        }else {
            int found = 0;
            int ni = 0;
            for(int j=0;j<10;j++) {
                if (str.text[i]==nums[j]) {
                    found = 1;
                    ni=j;
                    break;
                }
            }
            if (found) {
                if (!indecimal) {
                    float b = 1;
                    for (int j=0;j<idec-(i)-1;j++)
                        b*=10;
                    n += b*ni;
                }else {
                    float b = 1;
                    for (int j=0;j<i-idec;j++) {
                        b/=10;
                    }
                    n += b*ni;
                }
            }else {
                *err = 1;
                return 0;
            }
        }
    }
    return offset?-n:n;
}

double StringToDouble(String str,int* err) {
    char* nums = "0123456789";
    double n = 0;
    int indecimal = 0;
    int idec = 0;
    for(int i=0;i<str.size;i++) {
        if (str.text[i] == '.') {
            idec = i;
            break;
        }
    }
    int offset=0;
    if(str.text[0]=='-') {
        offset=1;
    }
    for (int i=offset;i<str.size;i++) {
        if (i == idec) {
            indecimal = 1;
        }else {
            int found = 0;
            int ni = 0;
            for(int j=0;j<10;j++) {
                if (str.text[i]==nums[j]) {
                    found = 1;
                    ni=j;
                    break;
                }
            }
            if (found) {
                if (!indecimal) {
                    double b = 1;
                    for (int j=0;j<idec-(i)-1;j++)
                        b*=10;
                    n += b*ni;
                }else {
                    double b = 1;
                    for (int j=0;j<i-idec;j++) {
                        b/=10;
                    }
                    n += b*ni;
                }
            }else {
                *err = 1;
                return 0;
            }
        }
    }
    return offset?-n:n;
}

String StringFromInt(int n,struct Arena* arena) {
    if(n == 0) {
        return StringFrom("0",arena);
    }
    char* nums = "0123456789";
    int offset = n<0;
    if (n<0)
        n = -n;
    float sizef = log10f((float)n);
    int size = ceilf(sizef);
    if(sizef==size) {
        size++;
    }
    char* text = ArenaAlloc(arena,(size+offset)*sizeof(char));
    String str = StringNew(text,size+offset);
    int i = 0;
    if (offset) {
        str.text[0] = '-';
    }
    while (n) {
        int r = n%10;
        str.text[offset+size-i-1] = nums[r];
        n/=10;
        i++;
    }
    return str;
}

String sfromint(int n) {
    return StringFromInt(n,&STRING_ARENA);
}

String StringFromLong(long n,struct Arena* arena) {
    if(n == 0) {
        return StringFrom("0",arena);
    }
    char* nums = "0123456789";
    int offset = n<0;
    if (n<0)
        n = -n;
    float sizef = log10f((float)n);
    int size = ceilf(sizef);
    if(sizef==size) {
        size++;
    }
    char* text = ArenaAlloc(arena,(size+offset)*sizeof(char));
    String str = StringNew(text,size+offset);
    long i = 0;
    if (offset) {
        str.text[0] = '-';
    }
    while (n) {
        long r = n%10;
        str.text[offset+size-i-1] = nums[r];
        n/=10;
        i++;
    }
    return str;
}

String sfromlong(long n) {
    return StringFromLong(n,&STRING_ARENA);
}

String StringFromFloat(float n,int afterPoint,struct Arena* arena) {
    String nstr = StringFromLong((long)n,arena);
    float dec = fabsf(n-(long)n);
    long ndec = floorf(dec*powl(10,afterPoint)+0.5);
    String decstr = StringFromLong(ndec,arena);
    String str = StringConcat(nstr,StringFrom(".",arena),arena);
    return StringConcat(str,decstr,arena);
}

String sfromfloat(float n,int afterPoint) {
    return StringFromFloat(n,afterPoint,&STRING_ARENA);
}

String StringFromDouble(double n,int afterPoint,struct Arena* arena) {
    String nstr = StringFromLong((long)n,arena);
    double dec = fabs(n-(long)n);
    long ndec = floor(dec*powl(10,afterPoint)+0.5);
    String decstr = StringFromLong(ndec,arena);
    String str = StringConcat(nstr,StringFrom(".",arena),arena);
    return StringConcat(str,decstr,arena);
}

String sfromdouble(double n,int afterPoint) {
    return StringFromDouble(n,afterPoint,&STRING_ARENA);
}


int StringFind(String s,String t,int start) {
    int index = -1;
    for(int i=start;i<s.size-t.size+1;i++) {
        int found = 1;
        for(int j=0;j<t.size && found;j++) {
            found = s.text[i+j]==t.text[j];
        }
        if (found) {
            index = i;
            break;
        }
    }
    return index;
}

StringArr StringSplit(String s, String on,int start,struct Arena* arena) {
    int size = 1;
    int i=StringFind(s,on,start);
    while(i>=0) {
        size++;
        i=StringFind(s,on,i+on.size);
    }
    StringArr arr;
    arr.size = size;
    arr.ptr = ArenaAlloc(arena,sizeof(String)*size);
    i = StringFind(s,on,start);
    int j = 0;
    int k = -on.size;
    while(i >= 0) {
        String sub = StringSub(s,k+on.size,i,arena);
        arr.ptr[j] = sub;
        j++;
        k=i;
        i=StringFind(s,on,i+on.size);
    }
    arr.ptr[arr.size-1] = StringSub(s,k+on.size,s.size-1,arena);
    return arr;
}

StringArr ssplit(String s,String on,int start) {
    return StringSplit(s,on,start,&STRING_ARENA);
}

String StringMerge(StringArr arr,String on,struct Arena* arena) {
    int size = 0;
    for(int i=0;i<arr.size;i++)
        size += arr.ptr[i].size;
    String str = StringAlloc(size,arena);
    int k = 0;
    for(int i=0;i<arr.size;i++) {
        String s = arr.ptr[i];
        for(int j=0;j<s.size;j++) {
            str.text[k] = s.text[j];
            k++;
        }
        if (i < arr.size-1) {
            for(int j=0;j<on.size;j++) {
                str.text[k] = on.text[j];
                k++;
            }
        }
    }
    return str;
}

String smerge(StringArr arr,String on) {
    return StringMerge(arr,on,&STRING_ARENA);
}

String StringFormat(struct Arena* arena,String format,...) {
    va_list args;
    va_start(args,format);
    int i = StringFind(format,StringFrom("%",arena),0);
    int j = 0;
    String str = StringFrom("",arena);
    while(i >= 0) {
        int size = 2;
        if (!(i > 0 && format.text[i-1] == '\\')) {
            str = StringConcat(str,StringSub(format,j,i,arena),arena);
            char c = format.text[i+1];
            switch (c) {
                case 'S':{
                    str = StringConcat(str,va_arg(args,String),arena);
                    break;}
                case 's':{
                    String sc = StringFrom(va_arg(args,char*),arena);
                    str = StringConcat(str,sc,arena);
                    break;}
                case 'c': {
                    String s = StringAlloc(1,arena);
                    s.text[0] = va_arg(args,int);
                    str = StringConcat(str,s,arena);
                    break;}
                case 'd':{
                    String d = StringFromInt(va_arg(args,int),arena);
                    str = StringConcat(str,d,arena);
                    break;}
                case 'f':{
                    float f = va_arg(args,double);
                    str = StringConcat(str,StringFromFloat(f,2,arena),arena);
                    break;}
                case 'l':{
                    size = 3;
                    char cnext = format.text[i+2];
                    switch(cnext) {
                        case 'd':{
                            str = StringConcat(str,StringFromInt(va_arg(args,long),arena),arena);
                            break;}
                        case 'f':{
                            str = StringConcat(str,StringFromDouble(va_arg(args,double),10,arena),arena);
                            break;}
                    }
                    break;}
            }
            j = i+size;
        }
        i = StringFind(format,StringFrom("%",arena),i+1);
    }
    va_end(args);
    str = StringConcat(str,StringSub(format,j,format.size,arena),arena);
    return str;
}

String sformat(String format,...) {
    struct Arena* arena = &STRING_ARENA;
    va_list args;
    va_start(args,format);
    int i = StringFind(format,StringFrom("%",arena),0);
    int j = 0;
    String str = StringFrom("",arena);
    while(i >= 0) {
        int size = 2;
        if (!(i > 0 && format.text[i-1] == '\\')) {
            str = StringConcat(str,StringSub(format,j,i,arena),arena);
            char c = format.text[i+1];
            switch (c) {
                case 'S':{
                    str = StringConcat(str,va_arg(args,String),arena);
                    break;}
                case 's':{
                    String sc = StringFrom(va_arg(args,char*),arena);
                    str = StringConcat(str,sc,arena);
                    break;}
                case 'c': {
                    String s = StringAlloc(1,arena);
                    s.text[0] = va_arg(args,int);
                    str = StringConcat(str,s,arena);
                    break;}
                case 'd':{
                    String d = StringFromInt(va_arg(args,int),arena);
                    str = StringConcat(str,d,arena);
                    break;}
                case 'f':{
                    float f = va_arg(args,double);
                    str = StringConcat(str,StringFromFloat(f,2,arena),arena);
                    break;}
                case 'l':{
                    size = 3;
                    char cnext = format.text[i+2];
                    switch(cnext) {
                        case 'd':{
                            str = StringConcat(str,StringFromInt(va_arg(args,long),arena),arena);
                            break;}
                        case 'f':{
                            str = StringConcat(str,StringFromDouble(va_arg(args,double),10,arena),arena);
                            break;}
                    }
                    break;}
            }
            j = i+size;
        }
        i = StringFind(format,StringFrom("%",arena),i+1);
    }
    va_end(args);
    str = StringConcat(str,StringSub(format,j,format.size,arena),arena);
    return str;
}

int StringEq(String a,String b) {
    if  (a.size != b.size) 
        return 0;
    for(int i=0;i<a.size;i++) {
        if (a.text[i]!=b.text[i]) {
            return 0;
        }
    }
    return 1;
}

String StringCpy(String str,struct Arena* arena) {
    String cpy = StringAlloc(str.size,arena);
    cpy.size = str.size;
    memcpy(cpy.text,str.text,str.size*sizeof(char));
    return cpy;
}

int* StringFindString(String str,int start,struct Arena* arena) {
    struct Arena scratch = ArenaCreate(1024);
    String quote = StringFrom("\"",&scratch);
    int istart= StringFind(str,quote,start);
    int i = istart;
    int iend = -1;
    int ok = 1;
    if(istart == -1)
        ok = 0;
    else {
        int found = 0;
        while(!found) {
            i = StringFind(str,quote,i+1);
            if(i == -1) {
                ok = 0;
                found = 1;
            }else {
                if (str.text[i-1] != '\\') {
                    found = 1;
                    iend = i;
                }
            }
        }
    }
    int* result = NULL;
    if (ok) {
        result = ArenaAlloc(arena,sizeof(int)*2);
        result[0] = istart+1;
        result[1] = iend;
    }
    ArenaDelete(&scratch);
    return result;
}

int* sFindString(String str,int start) {
    return StringFindString(str,start,&STRING_ARENA);
}

//finds first char instance inside a string and that isn't inside "" or '' and doesn't have a \ before it
int StringFindCharEscape(String str,char c,int start) {
    int found = 0;
    int insideStr = 0;
    for(int i=start;i<str.size && !found;i++) {
        char C = str.text[i];
        if(C == '\\')
            i++;
        else if(!insideStr) {
            if(C == c)
                return i;
            else if(C == '"')
                insideStr = 1;
            else if(C == '\'')
                insideStr = 2;
        }else if(insideStr == 1)
            insideStr = !(C == '"');
        else
            insideStr = !(C == '\'');
    }
    return -1;
}

int StringFindLast(String str, String src,int start) {
    int i = start;
    int j = i;
    while(i != -1) {
        j = i;
        i = StringFind(str,src,i+1);
    }
    return j;
}

String StringEscape(String str,struct Arena* arena) {
    int i,j;
    String new = StringFrom("",arena);

    // i = StringFind(str,StringFrom("\n",arena),0);
    // j = 0;
    // while(i != -1) {
    //     new = StringFormat(arena,StringFrom("%S%S\\\n",arena),new,StringSub(str,j,i-1,arena),arena);
    //     j = i+1;
    //     i = StringFind(str,StringFrom("\n",arena),i+1);
    // }
    // new = StringConcat(new,StringSub(str,j,str.size,arena),arena);

    i = StringFind(str,StringFrom("\0",arena),0);
    j = 0;
    
    while(i != -1) {
        new = StringFormat(arena,StringFrom("%S%S ",arena),new,StringSub(str,j,i,arena),arena);
        j = i+1;
        i = StringFind(str,StringFrom("\0",arena),i+1);
    }
    new = StringConcat(new,StringSub(str,j,str.size,arena),arena);
    /*
    i = StringFind(str,StringFrom("\r",arena),0);
    j = 0;
    
    while(i != -1) {
        new = StringFormat(arena,StringFrom("%S%S\\ ",arena),new,StringSub(str,j,i,arena),arena);
        j = i+1;
        i = StringFind(str,StringFrom("\r",arena),i+1);
    }
    new = StringConcat(new,StringSub(str,j,str.size,arena),arena);*/

    return new;
}

void StringResize(String* str) {
    int newsize = 0;
    for(char* c=(str->text);*c;c++)
        newsize++;
    str->size = newsize;
}

/*
int main(void) {
    struct Arena arena = StringInitArena();

    int erri;
    int n = StringToInt(Str("1400000"),&erri);

    String str = sfromint(n);
    printf("%d %s\n",n,stochar(str));

    int errf;
    double f = StringToDouble(Str("-30000000000000.141"),&errf);

    String sf = sfromdouble(f,10);

    printf("%lf %s\n",f,stochar(sf));

    StringArr arr = ssplit(Str("test/autre/test/tkt.txt"),Str("/"),0);
    for(int i=0;i<arr.size;i++) {
        printf("%d : %s\n",i,stochar(arr.ptr[i]));
    }

    String test = smerge(arr,Str(","));
    test = sconcat(Str("{"),test);
    test = sconcat(test,Str("}"));
    printf("%s\n",stochar(test));

    String fstr = sformat(Str("{%S,%s,%f}"),Str("yay"),"test",10.);

    printf("%s\n",stochar(fstr));

    ArenaDelete(&arena);
}*/
#endif