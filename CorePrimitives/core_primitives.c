#include <stdio.h>
#include "core_primitives.h"
#include <math.h>
#include <string.h>
#include "../vm/vm.h"

/* //////////////////////  TYPE MAP  ////////////////////// */

/* define the enum types to str mapping */
const char* PrimitiveTypeNames[] = {
    "int",
    "float",
    "bool",
    "str",
    "Null",
    "Invalid Object"
};

/* //////////////////////  CONSTRUCTORS ////////////////////// */

/* Constructor for int_Object */
int_Object* new_int(VM* vm, int64_t value) {
    // perform check in constant table
    if (value >= -510 && value <= 510) {
        PrimitiveObject *cached = vm->constants[3 + value + 510];
        if (cached) {
            // printf("using cached value\n");
            // printf("value: %ld\n", value);
            // printf("constant table: %ld mem addr: %p\n", ((int_Object *)cached)->value, cached);
            return (int_Object *)cached;
        }
    }

    int_Object* obj = (int_Object*)malloc(sizeof(int_Object));
    if (!obj) return NULL; // Handle allocation failure

    obj->base.vm = vm;
    obj->base.type = TYPE_int;
    obj->base.add = (BinaryOp)add_int;
    obj->base.mul = (BinaryOp)mul_int;
    obj->base.div = (BinaryOp)div_int;
    obj->base.mod = (BinaryOp)mod_int;
    obj->base.eq  = eq_int;
    obj->base.neq = neq_int;
    obj->base.geq = geq_int;
    obj->base.gt  = gt_int;
    obj->base.leq = leq_int;
    obj->base.lt  = lt_int;
    obj->base.__str__ = int_to_string;
    obj->value = (int64_t) value;
    obj->bwAND = (BinaryOp)bitwise_AND;
    obj->bwXOR = (BinaryOp)bitwise_XOR;
    obj->bwOR = (BinaryOp)bitwise_OR;
    obj->bwRSHIFT = (BinaryOp)bitwise_RSHIFT;
    obj->bwLSHIFT = (BinaryOp)bitwise_LSHIFT;
    return obj;
}

/* Constructor for float_Object */
float_Object* new_float(double value) {
    float_Object* obj = (float_Object*)malloc(sizeof(float_Object));
    if (!obj) return NULL;
    obj->base.type = TYPE_float;
    obj->base.add = (BinaryOp)add_float;
    obj->base.mul = (BinaryOp)mul_float;
    obj->base.div = (BinaryOp)div_float;
    obj->base.mod = (BinaryOp)mod_float;
    obj->base.eq  = eq_float;
    obj->base.neq = neq_float;
    obj->base.geq = geq_float;
    obj->base.gt  = gt_float;
    obj->base.leq = leq_float;
    obj->base.lt  = lt_float;
    obj->base.__str__= float_to_string;
    obj->value = (float) value;

    return obj;
}

/*
Constructor for bool_Object
Returns a pointer to bool_Object struct inheriting from PrimitiveObject.
Value of bool is equivilent to 1 (true) and 0 (false)
*/
bool_Object* new_bool(VM* vm, int bool_value) {
    // Normalize to 0 or 1
    int normalized = (bool_value != 0);

    // constants[1] -> false, constants[2] -> true
    int index = normalized ? 2 : 1;

    if (vm->constants[index]) {
        return (bool_Object *)vm->constants[index];  // Cached instance
    }

    bool_Object* obj = malloc(sizeof(bool_Object));
    if (!obj) return NULL;

    obj->base.vm = vm;
    obj->base.type = TYPE_bool;
    obj->base.add = add_bool;
    obj->base.mul = mul_bool;
    obj->base.div = NULL;  // not defined
    obj->base.mod = NULL;  // not defined
    obj->base.eq  = eq_bool;
    obj->base.neq = neq_bool;
    obj->base.geq = geq_bool;
    obj->base.gt  = gt_bool;
    obj->base.leq = leq_bool;
    obj->base.lt  = lt_bool;
    obj->base.__str__ = bool_to_string;
    obj->value = (int8_t)normalized;

    return obj;
}

/*
Constructor for str_Object
Returns a pointer to str_Object struct inheriting from PrimitiveObject.
Value holds a mutable char pointer.
*/
str_Object* new_str(const char* string_value) {
    str_Object* obj = (str_Object*)malloc(sizeof(str_Object));
    if (!obj) return NULL;
    obj->base.type = TYPE_str;
    obj->base.add = (BinaryOp)add_str;
    obj->base.mul = (BinaryOp)mul_str;
    obj->base.div = NULL;
    obj->base.mod = NULL;
    obj->base.eq  = eq_str;
    obj->base.neq = neq_str;
    obj->base.geq = geq_str;
    obj->base.gt  = gt_str;
    obj->base.leq = leq_str;
    obj->base.lt  = lt_str;
    obj->base.__str__ = str_to_string;
    obj->value = strdup(string_value); // This makes a copy of the string which always makes it mutable

    return obj;
}

/*
Singleton instance for Null_Object
*/
Null_Object* get_null(VM* vm) {
    if (vm->constants[0]) {
        return (Null_Object *)vm->constants[0];  // Cached singleton
    }

    Null_Object* obj = malloc(sizeof(Null_Object));
    if (!obj) return NULL;

    obj->base.vm = vm;
    obj->base.type = TYPE_Null;
    obj->base.add = NULL;
    obj->base.mul = NULL;
    obj->base.div = NULL;
    obj->base.mod = NULL;
    obj->base.eq = eq_NULL;
    obj->base.neq = neq_NULL;
    obj->base.geq = NULL;
    obj->base.gt  = NULL;
    obj->base.leq = NULL;
    obj->base.lt  = NULL;
    obj->base.__str__ = null_to_string;

    return obj;
}

/* //////////////////////  FUNC: FREE ////////////////////// */
/* universal free method for all primitives */
void free_primitive(PrimitiveObject* object) {
    if (!object) return; // Just to be safe

    switch (object->type) {
        case TYPE_str:
            free(((str_Object*)object)->value); // Free the allocated string
            break;
        case TYPE_bool:
        case TYPE_Null:
            return; // Also just to be safe just incase free is ever called on Null or Bool
        default:
            break;
    }

    free(object); // Free the object itself
} // keep in mind that this only frees the memory but does not set the ptr to null to prevent use after free


/* //////////////////////  PRIMITIVE OPERATORS  ////////////////////// */
/* //////////////////////  OPERATOR: add  ////////////////////// */

/* Add function for int_Object */
PrimitiveObject* add_int(PrimitiveObject* self, PrimitiveObject* other) {
    if (!self || !other) return NULL; // Null check

    switch (other->type) {
        case TYPE_float:
            return (PrimitiveObject*)new_float(((int_Object*)self)->value + ((float_Object*)other)->value);

        case TYPE_bool:
        case TYPE_int:
            return (PrimitiveObject*)new_int(self->vm, ((int_Object*)self)->value + ((int_Object*)other)->value);

        case TYPE_Null:
        case TYPE_str:
            printf("Addition not supported between %s and %s\n",
                   PrimitiveTypeNames[self->type], PrimitiveTypeNames[other->type]);
            return NULL; // Return NULL for more fine control over error handling
    }

    return NULL; // In case of unexpected type
}

/* Add function for float_Object */
PrimitiveObject* add_float(PrimitiveObject* self, PrimitiveObject* other) {
    if (!self || !other) return NULL;

    switch (other->type) {
        case TYPE_int:
            return (PrimitiveObject*)new_float(((float_Object*)self)->value + ((int_Object*)other)->value);

        case TYPE_float:
            return (PrimitiveObject*)new_float(((float_Object*)self)->value + ((float_Object*)other)->value);

        case TYPE_Null:
        case TYPE_str:
            printf("Addition not supported between %s and %s\n",
                   PrimitiveTypeNames[self->type], PrimitiveTypeNames[other->type]);
            return NULL;
    }

    return NULL;
}

/* Add function for bool_Object */
PrimitiveObject* add_bool(PrimitiveObject* self, PrimitiveObject* other) {
    if (!self || !other) return NULL;

    switch (other->type) {
        case TYPE_bool:
        case TYPE_int:
            return (PrimitiveObject*)new_int(self->vm, ((bool_Object*)self)->value + ((int_Object*)other)->value);

        case TYPE_float:
            return (PrimitiveObject*)new_float(((bool_Object*)self)->value + ((float_Object*)other)->value);

        case TYPE_Null:
        case TYPE_str:
            printf("Addition not supported between %s and %s\n",
                   PrimitiveTypeNames[self->type], PrimitiveTypeNames[other->type]);
            return NULL;
    }

    return NULL;

}

/* Add function for str_Object */
PrimitiveObject* add_str(PrimitiveObject* self, PrimitiveObject* other) {

    if (!self || !other) return NULL;

    if (other->type == TYPE_str) {
        char * str1 = ((str_Object *) self)->value;
        char * str2 = ((str_Object *) other)->value;

        size_t len1 = strlen(str1);
        size_t len2 = strlen(str2);
        if (len1 > SIZE_MAX - len2 - 1) { // Ensure that the concat operation is within bounds of a uint
            printf("Error: String concatenation size exceeds limit.\n");
            return NULL;
        }

        char * new_str_val = malloc(len1 + len2 + 1);
        if (!new_str_val) {
            printf("Memory allocation for string addition failed.\n");
            return NULL;
        }
        strcpy(new_str_val, str1);
        strcat(new_str_val, str2);
        str_Object * res = new_str(new_str_val); // constructor of str_Object duplicates new_str_val so must free it in next line
        free(new_str_val);                       // intended behaviour is for a new str to be instantiated when operators are applied onto it
        return (PrimitiveObject*)res;
    }

    printf("Addition not supported between %s and %s\n",
            PrimitiveTypeNames[self->type], PrimitiveTypeNames[other->type]);
    return NULL;
}

/* //////////////////////  OPERATOR: mul  //////////////////////// */

/* mul function for ints */
PrimitiveObject* mul_int(PrimitiveObject* self, PrimitiveObject* other) {
    if (!self || !other) return NULL; // Null check

    switch (other->type) {
        case TYPE_float:
            return (PrimitiveObject*)new_float(((int_Object*)self)->value * ((float_Object*)other)->value);

        case TYPE_bool:
            return (PrimitiveObject*)new_int(self->vm, ((int_Object*)self)->value * ((bool_Object*)other)->value);

        case TYPE_int:
            return (PrimitiveObject*)new_int(self->vm, ((int_Object*)self)->value * ((int_Object*)other)->value);

        case TYPE_str: {
            int i;
            int n = ((int_Object*)self)->value;

            if (n < 0) {
                printf("String multiplication not supported for negative numbers\n");
                return NULL;
            } else if (n == 0) {
                return (PrimitiveObject*)new_str(""); // Explicitly handle str * 0
            }

            char* str1 = ((str_Object*)other)->value;  // other is the string
            size_t len = strlen(str1) * n;
            char* new_str_val = malloc(len + 1);

            if (!new_str_val) {
                printf("Memory allocation for string multiplication failed.\n");
                return NULL;
            }

            new_str_val[0] = '\0';  // Initialize empty string
            for (i = 0; i < n; i++) {
                strcat(new_str_val, str1);
            }

            str_Object* res = new_str(new_str_val);
            free(new_str_val);  // Safe to free since new_str duplicates it
            return (PrimitiveObject*)res;
        }

        case TYPE_Null:
            printf("Multiplication not supported between %s and %s\n",
                   PrimitiveTypeNames[self->type], PrimitiveTypeNames[other->type]);
            return NULL;
    }

    return NULL;
}

/* mul function for floats */
PrimitiveObject* mul_float(PrimitiveObject* self, PrimitiveObject* other) {
    if (!self || !other) return NULL;

    switch (other->type) {
        case TYPE_float:
            return (PrimitiveObject*)new_float(((float_Object*)self)->value * ((float_Object*)other)->value);

        case TYPE_bool:
            return (PrimitiveObject*)new_float(((float_Object*)self)->value * ((bool_Object*)other)->value);

        case TYPE_int:
            return (PrimitiveObject*)new_float(((float_Object*)self)->value * ((int_Object*)other)->value);

        case TYPE_str:
        case TYPE_Null:
            printf("Multiplication not supported between %s and %s\n",
                   PrimitiveTypeNames[self->type], PrimitiveTypeNames[other->type]);
            return NULL;
    }

    return NULL;
}

/* mul function for bools */
PrimitiveObject* mul_bool(PrimitiveObject* self, PrimitiveObject* other) {
    if (!self || !other) return NULL;

    switch (other->type) {
        case TYPE_float:
            return (PrimitiveObject*)new_float(((bool_Object*)self)->value * ((float_Object*)other)->value);

        case TYPE_bool:
            return (PrimitiveObject*)new_int(self->vm, ((bool_Object*)self)->value && ((bool_Object*)other)->value);

        case TYPE_int:
            return (PrimitiveObject*)new_int(self->vm, ((bool_Object*)self)->value * ((int_Object*)other)->value);

        case TYPE_str:
            return (PrimitiveObject*)(((bool_Object*)self)->value ? new_str(((str_Object*)other)->value) : new_str(""));

        case TYPE_Null:
            printf("Multiplication not supported between %s and %s\n",
                   PrimitiveTypeNames[self->type], PrimitiveTypeNames[other->type]);
            return NULL;
    }

    return NULL;
}

/* mul function for strings */
PrimitiveObject* mul_str(PrimitiveObject* self, PrimitiveObject* other) {
    if (!self || !other) return NULL;

    switch (other->type) {
        case TYPE_bool:
            return other->mul(other, self); // Reuse bool's multiplication logic

        case TYPE_int:
            return other->mul(other, self); // Reuse int's multiplication logic

        case TYPE_float:
        case TYPE_str:
        case TYPE_Null:
            printf("Multiplication not supported between %s and %s\n",
                   PrimitiveTypeNames[self->type], PrimitiveTypeNames[other->type]);
            return NULL;
    }

    return NULL;
}

/* //////////////////////  OPERATOR: div  ////////////////////// */

/* div function for int */
PrimitiveObject* div_int(PrimitiveObject* self, PrimitiveObject* other) {
    if (!self || !other) return NULL;

    int_Object *a = (int_Object *)self;

    switch (other->type) {
        case TYPE_int: {
            int_Object * b = (int_Object *)other;
            if (b->value == 0) {
                printf("Error: Division by zero is not allowed.\n");
                return NULL;
            }
            if (a->value % b->value == 0) {
                return (PrimitiveObject *)new_int(self->vm, a->value / b->value);
            }
            return (PrimitiveObject *)new_float((double)a->value / b->value);
        }

        case TYPE_float: {
            float_Object * b = (float_Object *)other;
            if (b->value == 0.0) {
                printf("Error: Division by zero is not allowed.\n");
                return NULL;
            }
            return (PrimitiveObject *)new_float((double)a->value / b->value);
        }

        case TYPE_bool:
        case TYPE_str:
        case TYPE_Null:
            printf("Division not supported between %s and %s\n",
                   PrimitiveTypeNames[self->type], PrimitiveTypeNames[other->type]);
            return NULL;
    }

    return NULL;
}

/* div function for float */
PrimitiveObject* div_float(PrimitiveObject* self, PrimitiveObject* other) {
    if (!self || !other) return NULL;

    float_Object *a = (float_Object *)self;

    switch (other->type) {
        case TYPE_int: {
            int_Object *b = (int_Object *)other;
            if (b->value == 0) {
                printf("Error: Division by zero is not allowed.\n");
                return NULL;
            }
            return (PrimitiveObject *)new_float(a->value / b->value);
        }

        case TYPE_float: {
            float_Object *b = (float_Object *)other;
            if (b->value <= 0.0) {
                printf("Error: Division by zero or negative values are not allowed.\n");
                return NULL;
            }
            return (PrimitiveObject *)new_float(a->value / b->value);
        }

        case TYPE_bool:
        case TYPE_str:
        case TYPE_Null:
            printf("Division not supported between %s and %s\n",
                   PrimitiveTypeNames[self->type], PrimitiveTypeNames[other->type]);
            return NULL;
    }

    return NULL;
}

PrimitiveObject* mod_int(PrimitiveObject* self, PrimitiveObject* other) {
    if (!self || !other) return NULL;
    int_Object * a = (int_Object *) self;

    switch (other->type) { // for a % b, if b (other) is not an int the behaviour of modulo is undefined
        case TYPE_int:
            int_Object * b = (int_Object *) other;

            if (b->value <= 0) {
                printf("Error: Modulo by zero or negative values are not allowed.\n");
                return NULL;
            }

            return (PrimitiveObject *) new_int(self->vm, a->value % b->value);

        default:
            printf("Modulo not supported between %s and %s\n",
                    PrimitiveTypeNames[self->type], PrimitiveTypeNames[other->type]);
            return NULL;
    }

    return NULL;
}

PrimitiveObject* mod_float(PrimitiveObject* self, PrimitiveObject* other) {
    if (!self || !other) return NULL;
    float_Object * a = (float_Object *) self;

    switch (other->type) { // for a % b, if b (other) is not an int the behaviour of modulo is undefined
        case TYPE_int:
            int_Object * b = (int_Object *) other;

            if (b->value == 0) {
                printf("Error: Modulo by zero is not allowed.\n");
                return NULL;
            }

            return (PrimitiveObject *) new_float(a->value - b->value * floor(a->value/b->value));

        default:
            printf("Modulo not supported between %s and %s\n",
                    PrimitiveTypeNames[self->type], PrimitiveTypeNames[other->type]);
            return NULL;
    }

    return NULL;
}

/* //////////////////////  OPERATOR: bitwise  ////////////////////// */

PrimitiveObject* bitwise_XOR(PrimitiveObject* self, PrimitiveObject* other) {
    int_Object * a = (int_Object *) self;
    if (other->type == TYPE_int || other->type == TYPE_bool) {
        int_Object * b = (int_Object *) other; // We will treat bool (int_8) and int (int_64) as an int_object in this case as their values are prepresented by ints
        return (PrimitiveObject *) new_int(self->vm, a->value ^ b->value);
    }
    printf("Integer bitwise XOR not supported between %s and %s\n",
        PrimitiveTypeNames[self->type], PrimitiveTypeNames[(other->type >= TYPE_int && other->type <= TYPE_Null)? other->type:5 ]);
    return NULL;
}

PrimitiveObject* bitwise_AND(PrimitiveObject* self, PrimitiveObject* other) {
    int_Object * a = (int_Object *) self;
    if (other->type == TYPE_int || other->type == TYPE_bool) {
        int_Object * b = (int_Object *) other; // We will treat bool (int_8) and int (int_64) as an int_object in this case as their values are prepresented by ints
        return (PrimitiveObject *) new_int(self->vm, a->value & b->value);
    }
    printf("Integer bitwise XOR not supported between %s and %s\n",
        PrimitiveTypeNames[self->type], PrimitiveTypeNames[(other->type >= TYPE_int && other->type <= TYPE_Null)? other->type:5 ]);
    return NULL;
}

PrimitiveObject* bitwise_OR(PrimitiveObject* self, PrimitiveObject* other) {
    int_Object * a = (int_Object *) self;
    if (other->type == TYPE_int || other->type == TYPE_bool) {
        int_Object * b = (int_Object *) other; // We will treat bool (int_8) and int (int_64) as an int_object in this case as their values are prepresented by ints
        return (PrimitiveObject *) new_int(self->vm, a->value | b->value);
    }
    printf("Integer bitwise XOR not supported between %s and %s\n",
        PrimitiveTypeNames[self->type], PrimitiveTypeNames[(other->type >= TYPE_int && other->type <= TYPE_Null)? other->type:5 ]);
    return NULL;
}

PrimitiveObject* bitwise_RSHIFT(PrimitiveObject* self, PrimitiveObject* other) {
    int_Object * a = (int_Object *) self;
    if (other->type == TYPE_int || other->type == TYPE_bool) {
        int_Object * b = (int_Object *) other; // We will treat bool (int_8) and int (int_64) as an int_object in this case as their values are prepresented by ints
        return (PrimitiveObject *) new_int(self->vm, a->value >> b->value);
    }
    printf("Integer bitwise XOR not supported between %s and %s\n",
        PrimitiveTypeNames[self->type], PrimitiveTypeNames[(other->type >= TYPE_int && other->type <= TYPE_Null)? other->type:5 ]);
    return NULL;
}

PrimitiveObject* bitwise_LSHIFT(PrimitiveObject* self, PrimitiveObject* other) {
    int_Object * a = (int_Object *) self;
    if (other->type == TYPE_int || other->type == TYPE_bool) {
        int_Object * b = (int_Object *) other; // We will treat bool (int_8) and int (int_64) as an int_object in this case as their values are prepresented by ints
        return (PrimitiveObject *) new_int(self->vm, a->value << b->value);
    }
    printf("Integer bitwise XOR not supported between %s and %s\n",
        PrimitiveTypeNames[self->type], PrimitiveTypeNames[(other->type >= TYPE_int && other->type <= TYPE_Null)? other->type:5 ]);
    return NULL;
}

/* //////////////////////  OPERATOR: ==  ////////////////////// */

/*
Supported between all types.
computes equality for int, float, bool
returns (bool_Object false for NULL and str ALWAYS )
*/
int eq_int(PrimitiveObject* self, PrimitiveObject* other) {
    int_Object * a = (int_Object *) self;
    if (other->type == TYPE_int) {
        // printf("a: %ld\n", a->value);
        // printf("b: %ld\n", ((int_Object *) other)->value);
        // printf("TYPE INT triggered, result: %d\n",a->value == ((int_Object *) other)->value);
        return a->value == ((int_Object *) other)->value;

    } else if (other->type == TYPE_float) {

        return other->eq(other, self); // use the other's eq method if it is float as we need to have tolerance for floating point error

    } else if (other->type == TYPE_bool) {

        return a->value == ((bool_Object *) other)->value;

    } else {
        return 0; // return False for NULL and str comparisons
    }
}

/*

*/
int eq_float(PrimitiveObject* self, PrimitiveObject* other) {
    float_Object *a = (float_Object *)self;
    double b_value;

    switch (other->type) {
        case TYPE_float:
            b_value = ((float_Object *)other)->value;
            break;
        case TYPE_int:
            b_value = ((int_Object *)other)->value;
            break;
        case TYPE_bool:
            b_value = ((bool_Object *)other)->value;
            break;
        default:
            return 0;
    }

    double diff = fabs(a->value - b_value);
    double max_val = fmax(fabs(a->value), fabs(b_value));
    double epsilon = 1e-8; // tighter, but scales

    return diff <= epsilon * max_val;
}

int eq_bool(PrimitiveObject* self, PrimitiveObject* other) {
    int8_t a_val = ((bool_Object*)self)->value;

    switch (other->type) {
        case TYPE_bool:
            return a_val == ((bool_Object*)other)->value;
        case TYPE_int:
            return a_val == ((int_Object*)other)->value;
        case TYPE_float: 
            return other->eq(other,self);
        default:
            return 0; // False for str, null, etc.
    }
}

int eq_str(PrimitiveObject* self, PrimitiveObject* other) {
    if (other->type != TYPE_str) return 0;

    char* a_val = ((str_Object*)self)->value;
    char* b_val = ((str_Object*)other)->value;

    return strcmp(a_val, b_val) == 0;
}

int eq_NULL(PrimitiveObject* self, PrimitiveObject* other) {
    return other->type == TYPE_Null;
}

// /* //////////////////////  OPERATOR: >=  ////////////////////// */
int geq_int(PrimitiveObject* self, PrimitiveObject* other) {
    int64_t a = ((int_Object*)self)->value;

    switch (other->type) {
        case TYPE_int:
            return a >= ((int_Object*)other)->value;
        case TYPE_float:
            return (double)a >= ((float_Object*)other)->value;
        case TYPE_bool:
            return a >= ((bool_Object*)other)->value;
        default:
            return 0;
    }
}

int geq_float(PrimitiveObject* self, PrimitiveObject* other) {
    double a = ((float_Object*)self)->value;
    double b;

    switch (other->type) {
        case TYPE_float:
            b = ((float_Object*)other)->value;
            break;
        case TYPE_int:
            b = ((int_Object*)other)->value;
            break;
        case TYPE_bool:
            b = ((bool_Object*)other)->value;
            break;
        default:
            return 0;
    }

    return a >= b;
}

int geq_bool(PrimitiveObject* self, PrimitiveObject* other) {
    int8_t a = ((bool_Object*)self)->value;

    switch (other->type) {
        case TYPE_bool:
            return a >= ((bool_Object*)other)->value;
        case TYPE_int:
            return a >= ((int_Object*)other)->value;
        case TYPE_float:
            return other->eq(other, self) ? 1 : a > ((float_Object*)other)->value;
        default:
            return 0;
    }
}

int geq_str(PrimitiveObject* self, PrimitiveObject* other) {
    if (other->type != TYPE_str) return 0;

    char* a = ((str_Object*)self)->value;
    char* b = ((str_Object*)other)->value;

    return strcmp(a, b) >= 0;
}

// /* //////////////////////  OPERATOR: !=  ////////////////////// */
int neq_int(PrimitiveObject* self, PrimitiveObject* other) {
    return !eq_int(self, other);
}
int neq_float(PrimitiveObject* self, PrimitiveObject* other) {
    return !eq_float(self, other);
}
int neq_bool(PrimitiveObject* self, PrimitiveObject* other) {
    return !eq_bool(self, other);
}
int neq_str(PrimitiveObject* self, PrimitiveObject* other) {
    return !eq_str(self, other);
}
int neq_NULL(PrimitiveObject* self, PrimitiveObject* other) {
    return !eq_NULL(self, other);
}

// /* //////////////////////  OPERATOR: <=  ////////////////////// */
/* we can just negate gt to implement this, should've done this for geq too but oh well*/

int leq_int(PrimitiveObject* self, PrimitiveObject* other) {
    return !gt_int(self, other);
}

int leq_float(PrimitiveObject* self, PrimitiveObject* other) {
    return !gt_float(self, other);
}

int leq_bool(PrimitiveObject* self, PrimitiveObject* other) {
    return !gt_bool(self, other);
}

int leq_str(PrimitiveObject* self, PrimitiveObject* other) {
    return !gt_str(self, other);
}

// /* //////////////////////  OPERATOR: >  ////////////////////// */

int gt_int(PrimitiveObject* self, PrimitiveObject* other) {
    int64_t a = ((int_Object*)self)->value;

    switch (other->type) {
        case TYPE_int: return a > ((int_Object*)other)->value;
        case TYPE_float: return (double)a > ((float_Object*)other)->value;
        case TYPE_bool: return a > ((bool_Object*)other)->value;
        default: return 0;
    }
}
int gt_float(PrimitiveObject* self, PrimitiveObject* other) {
    double a = ((float_Object*)self)->value;
    double b;

    switch (other->type) {
        case TYPE_float: b = ((float_Object*)other)->value; break;
        case TYPE_int:   b = ((int_Object*)other)->value; break;
        case TYPE_bool:  b = ((bool_Object*)other)->value; break;
        default: return 0;
    }

    return a > b;
}
int gt_bool(PrimitiveObject* self, PrimitiveObject* other) {
    int8_t a = ((bool_Object*)self)->value;

    switch (other->type) {
        case TYPE_bool: return a > ((bool_Object*)other)->value;
        case TYPE_int:  return a > ((int_Object*)other)->value;
        case TYPE_float: return (double)a > ((float_Object*)other)->value;
        default: return 0;
    }
}

int gt_str(PrimitiveObject* self, PrimitiveObject* other) {
    if (other->type != TYPE_str) return 0;

    char* a = ((str_Object*)self)->value;
    char* b = ((str_Object*)other)->value;

    return strcmp(a, b) > 0;
}

// /* //////////////////////  OPERATOR: <  ////////////////////// */
int lt_int(PrimitiveObject* self, PrimitiveObject* other) {
    int64_t a = ((int_Object*)self)->value;

    switch (other->type) {
        case TYPE_int: return a < ((int_Object*)other)->value;
        case TYPE_float: return (double)a < ((float_Object*)other)->value;
        case TYPE_bool: return a < ((bool_Object*)other)->value;
        default: return 0;
    }
}

int lt_float(PrimitiveObject* self, PrimitiveObject* other) {
    double a = ((float_Object*)self)->value;
    double b;

    switch (other->type) {
        case TYPE_float: b = ((float_Object*)other)->value; break;
        case TYPE_int:   b = ((int_Object*)other)->value; break;
        case TYPE_bool:  b = ((bool_Object*)other)->value; break;
        default: return 0;
    }

    return a < b;
}

int lt_bool(PrimitiveObject* self, PrimitiveObject* other) {
    int8_t a = ((bool_Object*)self)->value;

    switch (other->type) {
        case TYPE_bool: return a < ((bool_Object*)other)->value;
        case TYPE_int:  return a < ((int_Object*)other)->value;
        case TYPE_float: return (double)a < ((float_Object*)other)->value;
        default: return 0;
    }
}

int lt_str(PrimitiveObject* self, PrimitiveObject* other) {
    if (other->type != TYPE_str) return 0;

    char* a = ((str_Object*)self)->value;
    char* b = ((str_Object*)other)->value;

    return strcmp(a, b) < 0;
}

// /* //////////////////////  __str__  ////////////////////// */
char* int_to_string(PrimitiveObject* obj) {
    int_Object* intObj = (int_Object*)obj;
    char* buffer = malloc(32);  // enough for 64-bit integers
    if (buffer) {
        snprintf(buffer, 32, "%ld", intObj->value);
    }
    return buffer;
}

char* float_to_string(PrimitiveObject* obj) {
    float_Object* floatObj = (float_Object*)obj;
    char* buffer = malloc(64);
    if (buffer) {
        snprintf(buffer, 64, "%lf", floatObj->value);
    }
    return buffer;
}

char* bool_to_string(PrimitiveObject* obj) {
    bool_Object* boolObj = (bool_Object*)obj;
    return strdup(boolObj->value ? "true" : "false");
}

char* null_to_string(PrimitiveObject* obj) {
    return strdup("NULL");
}

char* str_to_string(PrimitiveObject* obj) {
    str_Object* strObj = (str_Object*)obj;
    return strdup(strObj->value); //copy so we don't free the value of the primitive (as technically thats the job of the garbage collector)
}
