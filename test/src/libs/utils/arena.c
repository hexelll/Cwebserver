#ifndef MYARENA
#define MYARENA

#include "arena.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AUTO_SIZE_UP 1

struct Arena ArenaCreate(int size){
    struct Arena arena;
    arena.size = size;
    arena.start = malloc(size);
    arena.offset = 0;
    arena.next = 0;
    return arena;
}

void* ArenaAlloc(struct Arena* arena, int size){
    int s = arena->offset+size;
    struct Arena* curr = arena;
    if (AUTO_SIZE_UP) {
        while (curr->offset+size > curr->size) {
            if (!curr->next) {
                int newsize = curr->size > size ? curr->size : size*2;
                struct Arena next = ArenaCreate(newsize);
                curr->next = (struct Arena*)malloc(sizeof(struct Arena));
                *curr->next = next;
            }
            curr = curr->next;
        }
    }else if (s > arena->size) {
        perror("pointer out of range");
        exit(1);
    }
    void* ptr = curr->start+curr->offset;
    curr->offset += size;
    return ptr;
}

void ArenaDelete(struct Arena* arena) {
    struct Arena* curr = arena;
    while(curr->next) {
        curr = curr->next;
        struct Arena* oldptr = curr;
        free(curr->start);
        free(oldptr);
    }
    free(arena->start);
}
#endif