#include "ucache.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int*
Int(int x)
{
    int* i = malloc(sizeof(*i));
    *i = x;
    return i;
}

int
main(void)
{
    Cache* cache = Cache_Init(7);
    Cache_Put(cache, "A", Int(0));
    Cache_Put(cache, "B", Int(1));
    Cache_Put(cache, "C", Int(2));
    Cache_Put(cache, "D", Int(3));
    Cache_Put(cache, "E", Int(4));
    Cache_Put(cache, "F", Int(5));
    Cache_Put(cache, "G", Int(6));
    Cache_Put(cache, "G", Int(7)); /* Overwrites */
    Cache_Put(cache, "H", Int(8));
    Cache_Put(cache, "I", Int(9));

    /* Both A and B should be removed from the cache to make room for H and I */
    assert(Cache_Get(cache, "A") == NULL);
    assert(Cache_Get(cache, "B") == NULL);

    assert(*(int*) Cache_Get(cache, "C") == 2);
    assert(*(int*) Cache_Get(cache, "D") == 3);
    assert(*(int*) Cache_Get(cache, "E") == 4);
    assert(*(int*) Cache_Get(cache, "F") == 5);
    assert(*(int*) Cache_Get(cache, "G") == 7); /* Overwrite check */
    assert(*(int*) Cache_Get(cache, "H") == 8);
    assert(*(int*) Cache_Get(cache, "I") == 9);

    /* Reading C increases it's priority... */
    Cache_Get(cache, "C");

    /* Such that inserting Z will remove D */
    Cache_Put(cache, "Z", Int(9));
    assert(Cache_Get(cache, "D") == NULL);

    Cache_Kill(cache);
    return 0;
}
