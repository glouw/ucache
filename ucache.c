#include "ucache.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node Node;

typedef struct List List;

typedef struct Link Link;

typedef struct Hash Hash;

struct Node
{
    Node* next;
    Node* prev;
    char* key;
    void* value;
};

struct Link
{
    Link* next;
    Node* node;
};

struct Hash
{
    Link** link;
    uint64_t cap;
};

struct List
{
    Node* first;
    Node* last;
};

struct Cache
{
    Hash* hash;
    List* list;
    uint64_t size;
};

static
char*
Dups(char* string)
{
    return strcpy(malloc(strlen(string) +  1), string);
}

static
int
Equals(char* string, char* other)
{
    return strcmp(string, other) == 0;
}

static
uint64_t
DJB2(char* string)
{
    uint64_t hash = 5381;
    int c;
    while((c = *string++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

static
Node*
Node_Init(char* key, void* value)
{
    Node* self = malloc(sizeof(*self));
    self->next = NULL;
    self->prev = NULL;
    self->key = Dups(key);
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
Link*
Link_Init(Node* node)
{
    Link* self = malloc(sizeof(*self));
    self->next = NULL;
    self->node = node;
    return self;
}

static
void
Link_Kill(Link* self)
{
    free(self);
}

static
Hash*
Hash_Init(uint64_t cap)
{
    Hash* self = malloc(sizeof(*self));
    self->link = calloc(cap, sizeof(*self->link));
    self->cap = cap;
    return self;
}

static
void
Hash_Kill(Hash* self)
{
    uint64_t i;
    for(i = 0; i < self->cap; i++)
    {
        Link* link = self->link[i];
        while(link)
        {
            Link* next = link->next;
            Link_Kill(link);
            link = next;
        }
    }
    free(self->link);
    free(self);
}

static
uint64_t
Hash_Index(Hash* self, char* key)
{
    return DJB2(key) % self->cap;
}

static
void
Hash_Delete(Hash* self, char* key)
{
    uint64_t index = Hash_Index(self, key);
    Link* link = self->link[index];
    Link* prev = NULL;
    while(link)
    {
        if(Equals(link->node->key, key))
        {
            if(prev == NULL)
                self->link[index] = self->link[index]->next;
            else
                prev->next = link->next;
            Link_Kill(link);
            break;
        }
        prev = link;
        link = link->next;
    }
}

static
Node*
Hash_Get(Hash* self, char* key)
{
    uint64_t index = Hash_Index(self, key);
    Link* link = self->link[index];
    while(link)
    {
        if(Equals(link->node->key, key))
            return link->node;
        link = link->next;
    }
    return NULL;
}

static
void
Hash_Insert(Hash* self, Node* node)
{
    uint64_t index = Hash_Index(self, node->key);
    Link* link = Link_Init(node);
    link->next = self->link[index];
    self->link[index] = link;
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
List_Delete(List* self, Node* at)
{
    Node_Kill(List_Pop(self, at));
}

static
void
List_Promote(List* self, Node* at)
{
    List_Append(self, List_Pop(self, at));
}

Cache*
Cache_Init(uint64_t cap)
{
    if(cap == 0)
    {
        puts("error: ucache: Cache_Init(uint64_t cap): cap size cannot be 0");
        exit(1);
    }
    Cache* self = malloc(sizeof(*self));
    self->hash = Hash_Init(cap);
    self->list = List_Init();
    self->size = 0;
    return self;
}

void
Cache_Kill(Cache* self)
{
    Hash_Kill(self->hash);
    List_Kill(self->list);
    free(self);
}

static
void
Cache_Delete(Cache* self, Node* node)
{
    Hash_Delete(self->hash, node->key);
    List_Delete(self->list, node);
    self->size -= 1;
}

static 
void
Cache_Insert(Cache* self, Node* node)
{
    Hash_Insert(self->hash, node);
    List_Append(self->list, node);
    self->size += 1;
}

static
void
Cache_Pop(Cache* self)
{
    Cache_Delete(self, self->list->first);
}

static
int
Cache_Full(Cache* self)
{
    return self->size == self->hash->cap;
}

void
Cache_Put(Cache* self, char* key, void* value)
{
    Node* exists = Hash_Get(self->hash, key);
    if(exists)
        Cache_Delete(self, exists);
    else
    if(Cache_Full(self))
        Cache_Pop(self);
    Cache_Insert(self, Node_Init(key, value));
}

void*
Cache_Get(Cache* self, char* key)
{
    Node* at = Hash_Get(self->hash, key);
    if(at)
    {
        List_Promote(self->list, at);
        return at->value;
    }
    return NULL;
}
