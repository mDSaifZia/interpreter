#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdlib.h>
#include <stdint.h>

/* Define your HashmapEntry structure */
typedef struct HashmapEntry {
    char *key;
    void *value;
    struct HashmapEntry *next;
} HashmapEntry;

/* Define your Hashmap structure */
typedef struct Hashmap {
    HashmapEntry **table;
    size_t capacity;
    size_t length;
} Hashmap;

/* Function Declarations */
Hashmap* init_hashmap(size_t capacity);
void    hashmap_resize(Hashmap *hashmap);
void    hashmap_set(Hashmap *hashmap, const char *key, void *value);
void*   hashmap_get(Hashmap *hashmap, const char *key);
void    hashmap_delete(Hashmap *hashmap, const char *key, void (*free_value)(void*));
void    free_hashmap(Hashmap *hashmap, void (*free_value)(void*));
size_t  hash(const char* str, size_t capacity);

#endif