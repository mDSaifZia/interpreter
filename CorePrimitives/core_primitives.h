#ifndef CORE_PRIMITIVES
#define CORE_PRIMITIVES

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Define core primitive types */
typedef enum {
    TYPE_int,
    TYPE_float,
    TYPE_bool,
    TYPE_str,
    TYPE_Null,
} PrimitiveType;

/* maps the enum types to a string representation */
extern const char *PrimitiveTypeNames[];

/* Forward declaration */
typedef struct PrimitiveObject PrimitiveObject;

/* Function pointer type for binary operations */
typedef PrimitiveObject* (*BinaryOp)(PrimitiveObject*, PrimitiveObject*);

/* Base primitive object */
struct PrimitiveObject {
    PrimitiveType type;
    // void (*free)(PrimitiveObject* self); // all primitives except for null must be freed 
    BinaryOp add;
    BinaryOp mul;
    BinaryOp div;
    BinaryOp mod;
};

/* It is crucial that the primitive object base is the first feild in these derivative structs as it allows us
   to cast any sub-primitives to a primitive object and use its feilds. We will use this again later in high-order Objects*/

/* Integer object */
typedef struct int_Object {
    PrimitiveObject base;
    int64_t value;
    BinaryOp bwXOR;
    BinaryOp bwAND;
    BinaryOp bwOR;
    BinaryOp bwRSHIFT;
    BinaryOp bwLSHIFT;
} int_Object;

/* Float object */
typedef struct float_Object {
    PrimitiveObject base;
    double value;
} float_Object;

/* Boolean object */
typedef struct bool_Object {
    PrimitiveObject base;
    int8_t value;
} bool_Object;

/* String object */
typedef struct str_Object {
    PrimitiveObject base;
    char* value; // immutable
} str_Object;

/* Null object (singleton) */
typedef struct Null_Object {
    PrimitiveObject base;
} Null_Object;

/* Constructor functions */
int_Object* new_int(int64_t value);
float_Object* new_float(double value);
bool_Object* new_bool(int bool_value);
str_Object* new_str(const char* string_value);
Null_Object* get_null(); // Singleton instance 

/* Free functions */
void free_primitive(PrimitiveObject* object);

/* Operator functions */
PrimitiveObject* add_int(PrimitiveObject* self, PrimitiveObject* other);
PrimitiveObject* add_float(PrimitiveObject* self, PrimitiveObject* other);
PrimitiveObject* add_bool(PrimitiveObject* self, PrimitiveObject* other);
PrimitiveObject* add_str(PrimitiveObject* self, PrimitiveObject* other);

PrimitiveObject* mul_int(PrimitiveObject* self, PrimitiveObject* other);
PrimitiveObject* mul_float(PrimitiveObject* self, PrimitiveObject* other);
PrimitiveObject* mul_bool(PrimitiveObject* self, PrimitiveObject* other);
PrimitiveObject* mul_str(PrimitiveObject* self, PrimitiveObject* other);

PrimitiveObject* div_int(PrimitiveObject* self, PrimitiveObject* other);
PrimitiveObject* div_float(PrimitiveObject* self, PrimitiveObject* other);


PrimitiveObject* mod_int(PrimitiveObject* self, PrimitiveObject* other);
PrimitiveObject* mod_float(PrimitiveObject* self, PrimitiveObject* other);

/* Bitwise operators (only for accesible for int) */
PrimitiveObject* bitwise_XOR(PrimitiveObject* self, PrimitiveObject* other);
PrimitiveObject* bitwise_AND(PrimitiveObject* self, PrimitiveObject* other);
PrimitiveObject* bitwise_OR(PrimitiveObject* self, PrimitiveObject* other);
PrimitiveObject* bitwise_RSHIFT(PrimitiveObject* self, PrimitiveObject* other);
PrimitiveObject* bitwise_LSHIFT(PrimitiveObject* self, PrimitiveObject* other);



#endif
