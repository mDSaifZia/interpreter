#include <stdio.h>
#include "core_primitives.h"
#include <math.h>
#include <string.h>

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
int_Object* new_int(int64_t value) {
    int_Object* obj = (int_Object*)malloc(sizeof(int_Object));
    if (!obj) return NULL; // Handle allocation failure

    obj->base.type = TYPE_int;
    obj->base.add = (BinaryOp)add_int;
    obj->base.mul = (BinaryOp)mul_int;
    obj->base.div = (BinaryOp)div_int;
    obj->base.mod = (BinaryOp)mod_int;
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
    obj->value = (float) value;

    return obj;
}

/* 
Constructor for bool_Object
Returns a pointer to bool_Object struct inheriting from PrimitiveObject.
Value of bool is equivilent to 1 (true) and 0 (false) 
*/
bool_Object* new_bool(int bool_value) {
    bool_Object* obj = (bool_Object*)malloc(sizeof(bool_Object));
    if (!obj) return NULL;
    obj->base.type = TYPE_bool;
    obj->base.add = (BinaryOp)add_bool;
    obj->base.mul = (BinaryOp)mul_bool;
    obj->base.div = NULL; // Boolean division doesn't make sense (you can accidentally divide by 0 if bool is false)
    obj->base.mod = NULL; // Modulo is useless for booleans
    obj->value = (int8_t) (bool_value != 0); // Normalize to 1 (true) for any value otherwise 0 (false)
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
    obj->value = strdup(string_value); // This makes a copy of the string which always makes it mutable

    return obj;
}

/* 
Singleton instance for Null_Object
*/
Null_Object* get_null() {
    static Null_Object null_instance = {
        .base = { TYPE_Null, NULL, NULL, NULL, NULL}
    };

    return &null_instance; // Always return the same instance
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
            return (PrimitiveObject*)new_int(((int_Object*)self)->value + ((int_Object*)other)->value);

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
            return (PrimitiveObject*)new_int(((bool_Object*)self)->value + ((int_Object*)other)->value);

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
            return (PrimitiveObject*)new_int(((int_Object*)self)->value * ((bool_Object*)other)->value);

        case TYPE_int:
            return (PrimitiveObject*)new_int(((int_Object*)self)->value * ((int_Object*)other)->value);

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
            return (PrimitiveObject*)new_int(((bool_Object*)self)->value && ((bool_Object*)other)->value); 

        case TYPE_int:
            return (PrimitiveObject*)new_int(((bool_Object*)self)->value * ((int_Object*)other)->value);

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
                return (PrimitiveObject *)new_int(a->value / b->value);
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

            return (PrimitiveObject *) new_int(a->value % b->value);

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
        return (PrimitiveObject *) new_int(a->value ^ b->value);
    }
    printf("Integer bitwise XOR not supported between %s and %s\n", 
        PrimitiveTypeNames[self->type], PrimitiveTypeNames[(other->type >= TYPE_int && other->type <= TYPE_Null)? other->type:5 ]);
    return NULL;
}

PrimitiveObject* bitwise_AND(PrimitiveObject* self, PrimitiveObject* other) {
    int_Object * a = (int_Object *) self;
    if (other->type == TYPE_int || other->type == TYPE_bool) {
        int_Object * b = (int_Object *) other; // We will treat bool (int_8) and int (int_64) as an int_object in this case as their values are prepresented by ints
        return (PrimitiveObject *) new_int(a->value & b->value);
    }
    printf("Integer bitwise XOR not supported between %s and %s\n", 
        PrimitiveTypeNames[self->type], PrimitiveTypeNames[(other->type >= TYPE_int && other->type <= TYPE_Null)? other->type:5 ]);
    return NULL; 
}

PrimitiveObject* bitwise_OR(PrimitiveObject* self, PrimitiveObject* other) {
    int_Object * a = (int_Object *) self;
    if (other->type == TYPE_int || other->type == TYPE_bool) {
        int_Object * b = (int_Object *) other; // We will treat bool (int_8) and int (int_64) as an int_object in this case as their values are prepresented by ints
        return (PrimitiveObject *) new_int(a->value | b->value);
    }
    printf("Integer bitwise XOR not supported between %s and %s\n", 
        PrimitiveTypeNames[self->type], PrimitiveTypeNames[(other->type >= TYPE_int && other->type <= TYPE_Null)? other->type:5 ]);
    return NULL;
}

PrimitiveObject* bitwise_RSHIFT(PrimitiveObject* self, PrimitiveObject* other) {
    int_Object * a = (int_Object *) self;
    if (other->type == TYPE_int || other->type == TYPE_bool) {
        int_Object * b = (int_Object *) other; // We will treat bool (int_8) and int (int_64) as an int_object in this case as their values are prepresented by ints
        return (PrimitiveObject *) new_int(a->value >> b->value);
    }
    printf("Integer bitwise XOR not supported between %s and %s\n", 
        PrimitiveTypeNames[self->type], PrimitiveTypeNames[(other->type >= TYPE_int && other->type <= TYPE_Null)? other->type:5 ]);
    return NULL;
}

PrimitiveObject* bitwise_LSHIFT(PrimitiveObject* self, PrimitiveObject* other) {
    int_Object * a = (int_Object *) self;
    if (other->type == TYPE_int || other->type == TYPE_bool) {
        int_Object * b = (int_Object *) other; // We will treat bool (int_8) and int (int_64) as an int_object in this case as their values are prepresented by ints
        return (PrimitiveObject *) new_int(a->value << b->value);
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
    double delta;
    double epsilon = 0.0000001;
    float_Object * a = (float_Object *) self;

    if (other->type == TYPE_float) {

        delta = a->value - ((float_Object *) other)->value;
        return delta < epsilon;

    } else if (other->type == TYPE_int) {

        delta = a->value - ((int_Object *) other)->value;
        return delta < epsilon;

    } else if (other->type == TYPE_bool) {

        delta = a->value - ((bool_Object *) other)->value;
        return delta < epsilon;

    } else {
        return 0; 
    }
}

int eq_bool(PrimitiveObject* self, PrimitiveObject* other) {

}

int eq_str(PrimitiveObject* self, PrimitiveObject* other) {

}

int eq_NULL(PrimitiveObject* self, PrimitiveObject* other) {
    return (PrimitiveObject *) new_bool(other->type == TYPE_Null);
}

// /* //////////////////////  OPERATOR: >=  ////////////////////// */
// PrimitiveObject* geq_int(PrimitiveObject* self, PrimitiveObject* other) {

// }

// PrimitiveObject* geq_float(PrimitiveObject* self, PrimitiveObject* other) {

// }

// PrimitiveObject* geq_bool(PrimitiveObject* self, PrimitiveObject* other) {

// }

// PrimitiveObject* geq_str(PrimitiveObject* self, PrimitiveObject* other) {

// }

// /* //////////////////////  OPERATOR: !=  ////////////////////// */
// PrimitiveObject* neq_int(PrimitiveObject* self, PrimitiveObject* other);
// PrimitiveObject* neq_float(PrimitiveObject* self, PrimitiveObject* other);
// PrimitiveObject* neq_bool(PrimitiveObject* self, PrimitiveObject* other);
// PrimitiveObject* neq_str(PrimitiveObject* self, PrimitiveObject* other);
// PrimitiveObject* neq_NULL(PrimitiveObject* self, PrimitiveObject* other);

// /* //////////////////////  OPERATOR: <=  ////////////////////// */
// PrimitiveObject* leq_int(PrimitiveObject* self, PrimitiveObject* other) {

// }

// PrimitiveObject* leq_float(PrimitiveObject* self, PrimitiveObject* other) {

// }

// PrimitiveObject* leq_bool(PrimitiveObject* self, PrimitiveObject* other) {

// }

// PrimitiveObject* leq_str(PrimitiveObject* self, PrimitiveObject* other) {

// }

// /* //////////////////////  OPERATOR: >  ////////////////////// */
// PrimitiveObject* gt_int(PrimitiveObject* self, PrimitiveObject* other) {

// }

// PrimitiveObject* gt_float(PrimitiveObject* self, PrimitiveObject* other) {

// }

// PrimitiveObject* gt_bool(PrimitiveObject* self, PrimitiveObject* other) {

// }

// PrimitiveObject* gt_str(PrimitiveObject* self, PrimitiveObject* other) {

// }

// /* //////////////////////  OPERATOR: <  ////////////////////// */
// PrimitiveObject* lt_int(PrimitiveObject* self, PrimitiveObject* other) {

// }

// PrimitiveObject* lt_float(PrimitiveObject* self, PrimitiveObject* other) {

// }

// PrimitiveObject* lt_bool(PrimitiveObject* self, PrimitiveObject* other) {

// }

// PrimitiveObject* lt_str(PrimitiveObject* self, PrimitiveObject* other) {

// }