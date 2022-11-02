#ifndef __U_CACHE__
#define __U_CACHE__

#include <stdint.h>

typedef struct Cache Cache;

Cache*
Cache_Init(uint64_t cap); /* Works best with prime numbers! */

void
Cache_Kill(Cache*);

void
Cache_Put(Cache*, char* key, void* value);

void*
Cache_Get(Cache*, char* key);

#endif
