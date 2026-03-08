struct Arena {
    void* start;
    int size;
    int offset;
    struct Arena* next;
};

struct Arena ArenaCreate(int size);

void ArenaDelete(struct Arena* arena);

void* ArenaAlloc(struct Arena* arena, int size);