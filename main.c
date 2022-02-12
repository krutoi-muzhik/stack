#ifndef STACK_H_
#define STACK_H_
#include "stack.h"
#endif

int
main () {
	stack stk;
	StackConstruct (&stk);
	for (int i = 1; i < 10; i++) {
		StackPush (&stk, i);
		printf ("Pushed %d\n", i);
	}

	for (int i = 1; i < 10; i++) {
		printf ("Popped %d\n", StackPop (&stk));
	}

	// printf ("stk->size = %ld  got size = %ld\n", stk.size, GetStackSize (&stk));
	// printf ("stk->capacity = %ld  got capacity = %ld\n", stk.capacity, GetStackCapacity (&stk));

	//printf ("stk->hash = %lld  got hash = %lld\n", stk.hash, GetStackHash (&stk));
	//printf ("data hash = %lld  got hash = %lld\n", *((unsigned long long *) (stk.data + stk.capacity * sizeof (data_t))), GetDataHash (&stk));
	StackDestruct (&stk);

	return 0;
}
