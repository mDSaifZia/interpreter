#ifndef ADVANCED_PRIMITIVES
#define ADVANCED_PRIMITIVES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declaration */
typedef struct Object Object;

/* Structure for a function reference (points to bytecode in memory) */
typedef struct {
    char *name;       // Method name
    size_t bytecode_offset; // Bytecode location in memory/file
} MethodReference;

/* Structure for storing method references */
typedef struct {
    MethodReference **entries; // Array of method references
    size_t size;
    size_t capacity;
} MethodTable;

/* Object Base */
typedef struct {
    char *type;
} objectBase;

/* Object Structure */
struct Object {
    objectBase base;
    Object *parent;
    MethodTable *methods; // holds the method reference
};

/* Function prototypes */
Object *Object_new(const char *type, Object *parent);
void Object_add_method(Object *obj, const char *name, size_t bytecode_offset);
size_t Object_get_method(Object *obj, const char *name);
void Object_destroy(Object *obj);

#endif /* ADVANCED_PRIMITIVES */
