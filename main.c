#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node Node;

typedef struct List List;

struct Node
{
    Node* next;
    Node* prev;
    char* key;
    void* value;
};

struct List
{
    Node* first;
    Node* last;
};

static
Node*
Node_Init(char* key, void* value)
{
    Node* self = malloc(sizeof(*self));
    self->next = NULL;
    self->prev = NULL;
    self->key = strdup(key);
    self->value = value;
    return self;
}

static
void
Node_Kill(Node* self)
{
    free(self->key);
    free(self->value);
    free(self);
}

static
List*
List_Init(void)
{
    List* self = malloc(sizeof(*self));
    self->first = NULL;
    self->last = NULL;
    return self;
}

static inline
int
List_Empty(List* self)
{
    return self->first == NULL && self->last == NULL;
}

static
void
List_Append(List* self, Node* node)
{
    if(List_Empty(self))
    {
        self->first = node;
        self->last = node;
    }
    else
    {
        self->last->next = node;
        node->prev = self->last;
        self->last = node;
    }
}

static
Node*
List_Pop(List* self, Node* at)
{
    if(List_Empty(self))
    {
        puts("List_Pop(List*, Node*) :: tried to pop an empty list");
        exit(1);
    }
    else
    if(self->first == self->last)
    {
        self->first = NULL;
        self->last = NULL;
    }
    else
    if(at == self->last)
    {
        self->last = self->last->prev;
        self->last->next = NULL;
    }
    else
    if(at == self->first)
    {
        self->first = self->first->next;
        self->first->prev = NULL;
    }
    else
    {
        at->prev->next = at->next;
        at->next->prev = at->prev;
    }
    at->prev = NULL;
    at->next = NULL;
    return at;
}

static
void
List_Delete(List* self)
{
    Node_Kill(List_Pop(self, self->first));
}

static
void
List_Promote(List* self, Node* at)
{
    List_Append(self, List_Pop(self, at));
}

static
void
List_Kill(List* self)
{
    Node* at = self->first;
    while(at)
    {
        Node* next = at->next;
        Node_Kill(at);
        at = next;
    }
    free(self);
}

static
void
List_Print(List* self)
{
    Node* at = self->first;
    while(at)
    {
        printf("%20p %10s %20p\n", at->prev, at->key, at->next);
        at = at->next;
    }
}

int
main(void)
{
    List* list = List_Init();
    List_Append(list, Node_Init("A", NULL));
    List_Append(list, Node_Init("B", NULL));
    List_Append(list, Node_Init("C", NULL));
    List_Promote(list, list->last);
    List_Print(list);
    List_Kill(list);
}
