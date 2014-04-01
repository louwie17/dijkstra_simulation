/*
 * File:     Stack.c
 * Author:   polaris8920
 * Date:     2013/04/13
 * Version:  1.0
 *
 * Purpose:
 * Create a module which provides basic stack functions.
 */

#include    <stdio.h>
#include    <stdlib.h>
#include    "Stack.h"

#define     AVOID(stmt, block)	    { if (stmt) {block;} }
#define     NULL_CHECK(ptr)	    { if (ptr == NULL) {return STACK_ERROR;} }

#define     INIT_SIZE		    (10)
#define     GROWTH_FACTOR	    (2)
#define     SHRINK_THRESHOLD	    (3)  /* if list is only
					  * 1 / THRESHOLD full, then shrink
					  * by GROWTH_FACTOR */

/**************************************************************************
 * Struct definitions
 *************************************************************************/
struct Stack
{
    int * data;
    int allocated;
    int used;
};

/**************************************************************************
 * Stack helper functions
 *************************************************************************/
static
int
stack_resize(StackT stack)
{
    NULL_CHECK(stack);
    int * newData = realloc(stack->data,
			    stack->used * GROWTH_FACTOR * sizeof(int));
    NULL_CHECK(newData);
    stack->data = newData;
    stack->allocated = stack->used * GROWTH_FACTOR;
    return STACK_SUCCESS;
}

/**************************************************************************
 * Stack functions
 *************************************************************************/
StackT
stack_new()
{
    StackT newStack = malloc(sizeof(struct Stack));
    AVOID(newStack == NULL, return NULL);
    newStack->data = malloc(sizeof(int) * INIT_SIZE);
    AVOID(newStack->data == NULL, stack_destroy(&newStack); return NULL);
    newStack->allocated = INIT_SIZE;
    newStack->used = 0;
    return newStack;
}

int
stack_destroy(StackT * stackPtr)
{
    NULL_CHECK(stackPtr);
    NULL_CHECK(*stackPtr);
    free((*stackPtr)->data);
    free(*stackPtr);
    *stackPtr = NULL;
    return STACK_SUCCESS;
}

int
stack_size(StackT stack)
{
    NULL_CHECK(stack);
    return stack->used;
}

int
stack_push(StackT stack, int aValue)
{
    NULL_CHECK(stack);
    NULL_CHECK(stack->data);
    if (stack->used >= stack->allocated)
	AVOID(stack_resize(stack) == STACK_ERROR, return STACK_ERROR);
    stack->data[++stack->used - 1] = aValue;
    return STACK_SUCCESS;
}

int stack_pop(StackT stack, int * dropoff)
{
    NULL_CHECK(stack);
    AVOID(stack->used <= 0, return STACK_ERROR);
    int tmp = stack->data[stack->used - 1];
    if (stack->used * SHRINK_THRESHOLD < stack->allocated)
	AVOID(stack_resize(stack) == STACK_ERROR, return STACK_ERROR);
    if (dropoff != NULL)
	*dropoff = tmp;
    stack->used--;
    return STACK_SUCCESS;
}

int stack_print(StackT stack)
{
    NULL_CHECK(stack);
    int i = stack_size(stack) - 1;
    printf("Stack (%d/%d Used/Allocated)\n", stack->used, stack->allocated);
    printf("Top: ");
    for (; i >= 0; i--)
	printf("%d ", stack->data[i]);
    printf("\n");    
    return STACK_SUCCESS;
}
