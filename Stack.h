#ifndef STACK_H
#define STACK_H

#define     STACK_ERROR   (-2)
#define     STACK_SUCCESS (0)

typedef struct Stack * StackT;

StackT stack_new();
int stack_destroy(StackT *);

int stack_push(StackT, int);
int stack_pop(StackT, int *);
int stack_size(StackT);

int stack_print(StackT);
#endif
