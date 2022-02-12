#pragma once

#include <stdio.h>
#include <wchar.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

typedef int data_t;
typedef int canary_t;

typedef struct stack_struct {
	canary_t canary1;
	size_t capacity;
	size_t size;
	char *data;
	unsigned long long hash;
	canary_t canary2;
} stack;

enum Errors {
	NO_ERRORS = 0,
	NULL_POINTER = 1,
	CANARY_KILLED = 2,
	DATA_INVASION = 3,
	SIZE_ERROR = 4,
	CAPACITY_ERROR = 5,
	STACK_HASH_ERROR = 6,
	DATA_HASH_ERROR = 7,
	STACK_EMPTE = 8
};

// ToDo: global const - write in CAPS

unsigned long long GetCRC (char* data, size_t start, size_t end);
unsigned long long GetDataHash (stack* stk);
unsigned long long GetStackHash (stack* stk);
void StackConstruct (stack *stk);
void StackDestruct (stack *stk);
void StackSizeUp (stack *stk);
void StackSizedDown (stack *stk);
void StackPush (stack *stk, data_t value);
data_t StackPop (stack *stk);
size_t GetStackSize (stack *stk);
size_t GetStackCapacity (stack *stk);
int StackError (stack *stk);
char *Timenow ();
void PrintStackLogs (stack *stk);
void Dump (stack *stk, int error, const char *function, const int line);
