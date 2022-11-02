# Micro Cache (ucache)

Micro cache stores key value pairs using using a simplified LRU cache.

Keys are strings and are copied on use. Values are not copied but will
be freed once the cache is freed.

The LRU is compromised of a hash map with singly linked bucketed nodes
for O(1) read and insertion and a doubly linked list for maintaining
deletion order, in that, the cache does not grow beyond capacity -
least used key value pairs are deleted to make room for new key value
insertions.
