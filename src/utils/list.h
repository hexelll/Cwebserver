#include "arena.c"
#include "string.c"

typedef struct {
    void* ptr;
}ListValue;

typedef struct ListNode {
    ListValue val;
    struct ListNode* next;
}ListNode;

typedef struct {
    struct Arena* arena;
    ListNode head;
    int size;
}List;

List ListNew(struct Arena* arena);

void ListAppendNode(List* list,ListNode node);

void ListAppendVal(List* list,ListValue val);

ListNode* ListPopNode(List* list);

ListValue* ListPopVal(List* list);

ListNode* ListGetNode(List* list,int index);

ListValue* ListGetVal(List* list,int index);

void ListSetVal(List* list,int index,ListValue val);

void ListInsertNode(List* list,int index,ListNode node);

void ListInsertVal(List* list,int index,ListValue val);

ListNode* ListRemoveNode(List* list,int index);
