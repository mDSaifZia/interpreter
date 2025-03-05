#include "core_primitives.h"
#include <stdlib.h>
#include <stdio.h>

void print_result(PrimitiveObject* result) {
    if (!result) {
        printf("Operation resulted in NULL (unsupported or error).\n");
        return;
    }

    switch (result->type) {
        case TYPE_int:
            printf("Result: %ld (int)\n", ((int_Object*)result)->value);
            break;
        case TYPE_float:
            printf("Result: %lf (float)\n", ((float_Object*)result)->value);
            break;
        case TYPE_bool:
            printf("Result: %s (bool)\n", ((bool_Object*)result)->value ? "true" : "false");
            break;
        case TYPE_str:
            printf("Result: \"%s\" (string)\n", ((str_Object*)result)->value);
            break;
        case TYPE_Null:
            printf("Result: Null\n");  
            return;
    }
    free_primitive(result);
}

int main(int argc, char const *argv[])
{
    /* Create primitive objects */
    PrimitiveObject* int0 = (PrimitiveObject *) new_int(0);
    PrimitiveObject* int1 = (PrimitiveObject*)new_int(10);
    PrimitiveObject* int2 = (PrimitiveObject*)new_int(4);
    PrimitiveObject* float1 = (PrimitiveObject*)new_float(5.5);
    PrimitiveObject* float2 = (PrimitiveObject*)new_float(2.0);
    PrimitiveObject* bool_true = (PrimitiveObject*)new_bool(100);
    PrimitiveObject* bool_false = (PrimitiveObject*)new_bool(0);
    PrimitiveObject* str1 = (PrimitiveObject*)new_str("Hello");
    PrimitiveObject* str2 = (PrimitiveObject*)new_str(" World");
    PrimitiveObject* null_obj = (PrimitiveObject*)get_null();

    /* Test Addition */
    printf("\n=== Addition Tests ===\n");
    print_result(int1->add(int1, int2));       // 10 + 4
    print_result(int1->add(int1, float1));     // 10 + 5.5
    print_result(float1->add(float1, float2)); // 5.5 + 2.0
    print_result(bool_true->add(bool_true, int1)); // true + 10
    print_result(str1->add(str1, str2));       // "Hello" + " World"
    print_result(int1->add(int1, null_obj));   // 10 + Null (should fail)

    /* Test Multiplication */
    printf("\n=== Multiplication Tests ===\n");
    print_result(int1->mul(int1, int2));       // 10 * 4
    print_result(int1->mul(int1, float1));     // 10 * 5.5
    print_result(float1->mul(float1, float2)); // 5.5 * 2.0
    print_result(bool_true->mul(bool_true, int1)); // true * 10
    print_result(str1->mul(str1, int2));       // "Hello" * 4
    print_result(int2->mul(int2, str1));       // 4 * "Hello"
    print_result(str1->mul(str1, int0));       // "Hello" * 0
    print_result(int0->mul(int0, str1));       // 0 * "Hello"
    print_result(((PrimitiveObject *)new_int(1))->mul(new_int(1), str1)); // 1 * "Hello"
    print_result(str2->mul(str2, bool_true)); // "World" * true
    print_result(bool_true->mul(bool_true, str2)); // true * "World"
    print_result(str2->mul(str2, bool_false)); // "World" * false
    print_result(bool_false->mul(bool_false, str2)); // false * "World"
    print_result(int1->mul(int1, null_obj));   // 10 * Null (should fail)

    /* Test Division */
    printf("\n=== Division Tests ===\n");
    print_result(int1->div(int1, int2));       // 10 / 4
    print_result(int1->div(int1, int1));       // 10 / 10
    print_result(int1->div(int1, float1));     // 10 / 5.5
    print_result(float1->div(float1, float2)); // 5.5 / 2.0
    print_result(int1->div(int1, null_obj));   // 10 / Null (should fail)
    print_result(int1->div(int1, int0)); // 10 / 0 (should fail)

    /* Test Modulo */
    printf("\n=== Modulo Tests ===\n");
    print_result(int1->mod(int1, int2));       // 10 % 4
    print_result(int1->mod(int1, int1));       // 10 % 10
    print_result(float1->mod(float1,new_int(1))); // 5.5 % 1
    print_result(int1->mod(int1, float1));     // 10 % 5.5 (should fail)
    print_result(float1->mod(float1, float2)); // 5.5 % 2.0 (should fail) (we don't support modulo by float at all)
    print_result(int1->mod(int1, null_obj));   // 10 % Null (should fail)
    print_result(int1->mod(int1, int0)); // 10 % 0 (should fail)

    /* Cleanup memory */
    free_primitive(int1);
    free_primitive(int2);
    free_primitive(float1);
    free_primitive(float2);
    free_primitive(bool_true);
    free_primitive(bool_false);
    free_primitive(str1);
    free_primitive(str2);
    free_primitive(int0);
    return 0;
}
