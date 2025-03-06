#ifndef ADVANCED_PRIMITIVES
#define ADVANCED_PRIMITIVES


/* foward delcaration */
typedef struct objectBase objectBase;
typedef struct Object Object;

/* Function pointer type for "dunder methods" */
typedef Object * (*dunder_operator) (Object*, Object*);
typedef Object * (*dunder_method) (Object*);

/* baseObject */



#endif