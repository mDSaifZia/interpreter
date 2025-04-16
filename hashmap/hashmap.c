#include "hashmap.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ///////////////////////// HASHMAP ///////////////////////// */
Hashmap * init_hashmap(size_t capacity) {
    Hashmap * hashmap = malloc(sizeof(Hashmap));
    if (! hashmap) { // ensure that we can allocate space for the hashmap
        printf("Hashmap allocation failed\n");
        return NULL;
    }
    hashmap->table = calloc(capacity, sizeof(HashmapEntry*)); // allocate the size of the hashmap pointer (we use calloc here because we do not want random junk in the pointer table)
    hashmap->length = 0;
    hashmap->capacity = capacity;
    return hashmap;
}

size_t hash(const char* str, size_t capacity) {
    size_t hash = 4123;
    while (*str) {
        hash = ((hash << 5) + hash) + (*str++);
    }
    return hash % capacity;
}

void hashmap_resize(Hashmap * hashmap, void (*free_value)(void*)) {
    // temporaily save the old capacity and old table
    size_t old_capacity = hashmap->capacity;
    HashmapEntry **old_table = hashmap->table;
    
    // double the capacity of the hashmap
    hashmap->capacity *= 2;
    hashmap->table = calloc(hashmap->capacity, sizeof(HashmapEntry*));
    hashmap->length = 0;

    if (!hashmap->table) {
        printf("Hashmap resize failed: Memory allocation error.\n");
        hashmap->table = old_table;
        hashmap->capacity = old_capacity;
        return;
    }
    
    // Begin rehashing to new table and free the old table
    size_t i;
    for (i=0; i < old_capacity; i++) {
        HashmapEntry * entry = old_table[i]; 
        while (entry) { 
            hashmap_set(hashmap, entry->key, entry->value, free_value);

            free(entry->key);  // free old key as hashmap_set duplicates it
            free(entry);
            entry = entry->next;
        }
    }

    free(old_table);
}

// unint32_t is as used since we can resonably expect that there will not be more than 2^32 variables declared by any single running program.

/*
Sets a char * "key" to a specified void * "value". DOES NOT free the key.
key needs to be explictly freed after calling hashmap_set
*/
void hashmap_set(Hashmap * hashmap, const char * key, void * value, void (*free_value)(void*)) {
    // Check if the hashmap occupancy exceeds the tolerance 
    float tolerance = 0.85;
    float occupancy = (float) hashmap->length/ (float) hashmap->capacity;
    if (occupancy > tolerance) {
        hashmap_resize(hashmap, free_value);
    }

    size_t index = hash(key, hashmap->capacity);
    HashmapEntry * entry = hashmap->table[index];

    // check if a key exists:
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (free_value) {
                free_value(entry->value);  // free if needed.
            }
            entry->value = value;
            return;
        }
        entry = entry->next; // key does not exist and hash collision has occured start a new entry
    }

    // add in the new entry
    entry = malloc(sizeof(HashmapEntry));
    entry->key = strdup(key);
    entry->value = value;
    entry->next = hashmap->table[index]; // As we calloced the memory in the table, if this is the first entry .next will be set to NULL, else it will be become the head of the previous hashmap entries
    hashmap->table[index] = entry; // set the entry head as the new pointer reference
    hashmap->length++; // increment the count to keep track of length of hashmap (not really used but good to have)
}

/*
Gets the value of char * key. DOES NOT free the key. 
key needs to explicitly freed after calling hashmap_get
*/
void * hashmap_get(Hashmap * hashmap, const char * key) {
    size_t index = hash(key, hashmap->capacity);
    HashmapEntry * entry = hashmap->table[index];

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;    // remember that we are returning a void pointer so it must be cast and handled appropriately
        }
        entry = entry->next; 
    }
    return NULL; // if value is not found
}

// When used in global table, we will not expect to be deleting any variables
// void (*free_value) (void*) is a function pointer to any function that will be used to free the value as we are storing void pointers which would have to be handled individually
void hashmap_delete(Hashmap * hashmap, const char * key, void (*free_value)(void*)) { 
    size_t index = hash(key, hashmap->capacity);
    HashmapEntry * entry = hashmap->table[index];
    HashmapEntry * prev = NULL;

    while (entry) {
        // handle linked list reattachment if possible
        if (strcmp(entry->key, key) == 0) { // if there is a key match
            if (prev) {
                prev->next = entry->next; // if the entry is a middle value in the linked list re-attach the ends (should have made a doubly linked list)
            } else {
                hashmap->table[index] = entry->next; // if the entry to be removed is the first (or only) entry just set its next value as the new head (will be NULL pointer if it is the onlyt entry)
            }
            if (free_value) { // only run this if given a function pointer for freeing certain values. (if the value stored does not need to be freedm NULL will be passed into the argument)
                free_value(entry->value);
            }
            free(entry->key); // free the string that was used as the name
            free(entry); // free the entry itself
            hashmap->length--;
            return;
        }
        // key has not been found yet
        prev = entry; // keep track of last checked entry
        entry = entry->next;
    }
    printf("Key \"%s\" does not exist.\n", key); // return NULL to indicate that a key cannot be found
    return;
}

/* Hashmap should not need to be freed in the case of the global table as it is never expected to be resized */
void free_hashmap(Hashmap * hashmap, void (*free_value)(void*)) {
    size_t i;
    for (i=0; i < hashmap->capacity; i++) {
        HashmapEntry * entry = hashmap->table[i]; 
        while (entry) {        
            if (free_value){
                free_value(entry->value);
            }
            free(entry->key);
            free(entry);
            entry = entry->next;
        }
    }
    free(hashmap->table);
    free(hashmap);
}
/* ///////////////////////// HASHMAP ///////////////////////// */