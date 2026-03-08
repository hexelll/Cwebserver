#ifndef MYLINKEDLIST
#define MYLINKEDLIST
#include "list.h"

List ListNew(struct Arena* arena) {
    ListNode node = {
        .val={.ptr=NULL},
        .next=NULL
    };
    return(List){
        .head=node,
        .size=0,
        .arena=arena
    };
}

void ListAppendNode(List* list,ListNode node) {
    if(list->size == 0) {
        list->head = node;
        list->size++;
        return;
    }
    ListNode* curr = &list->head;
    for(int i=0;i<list->size-1;i++)
        curr = curr->next;
    curr->next = (ListNode*)ArenaAlloc(list->arena,sizeof(ListNode));
    *curr->next = node;
    list->size++;
}

void ListAppendVal(List* list,ListValue val){
    ListAppendNode(list,(ListNode){.val=val,.next=NULL});
}

ListNode* ListPopNode(List* list) {
    if (list->size == 0) {
        return NULL;
    }
    ListNode* curr = &list->head;
    for(int i=0;i<list->size-2;i++)
        curr = curr->next;
    ListNode* popped = curr->next;
    curr->next = NULL;
    list->size--;
    return popped;
}

ListValue* ListPopVal(List* list) {
    ListNode* node = ListPopNode(list);
    if(!node) return NULL;
    return &node->val;
}

ListNode* ListGetNode(List* list,int index) {
    ListNode* curr = &list->head;
    if(index >= list->size) {
        return NULL;
    }
    for(int i=0;i<index;i++)
        curr = curr->next;
    return curr;
}

ListValue* ListGetVal(List* list,int index) {
    return &ListGetNode(list,index)->val;
}

void ListSetVal(List* list,int index,ListValue val) {
    ListGetNode(list,index)->val = val;
}

void ListInsertNode(List* list,int index,ListNode node) {
    ListNode* nodeI;
    if (index == 0) {
        nodeI = &list->head;
        node.next = nodeI;
        list->head = node;
        list->size++;
        return;
    }
    nodeI = ListGetNode(list,index-1);
    ListNode* next = nodeI->next;
    nodeI->next = ArenaAlloc(list->arena,sizeof(ListNode));
    node.next = next;
    *nodeI->next = node;
    list->size++;
}

void ListInsertVal(List* list,int index,ListValue val) {
    ListNode node = {
        .val = val,
        .next=NULL
    };
    ListInsertNode(list,index,node);
}

ListNode* ListRemoveNode(List* list,int index) {
    if(index==0) {
        ListNode* node = &list->head;
        list->head = *list->head.next;
        list->size--;
        return node;
    }
    ListNode* prev = ListGetNode(list,index-1);
    ListNode* node = prev->next;
    prev->next = node?node->next:NULL;
    list->size--;
    return node;
}


/*
int main() {
    struct Arena arena = ArenaCreate(1024);
    List list = ListNew(&arena);
    for(int i=0;i<100;i++) {
        int* a = ArenaAlloc(&arena,sizeof(int));
        *a = i;
        ListAppendVal(&list,Val(a,sizeof(int*)));
    }
    for(int i=0;i<5;i++) {
        ListPopNode(&list);
    }
    ListInsertVal(&list,list.size-1,Val(&(int){4},sizeof(int*)));
    ListInsertVal(&list,list.size-1,Val(&(int){400},sizeof(int*)));
    ListRemoveNode(&list,0);
    ListRemoveNode(&list,1);
    for(int i=0;i<list.size;i++) {
        printf("%d\n",*(int*)ListGetVal(&list,i)->ptr);
    }
}
*/

#endif