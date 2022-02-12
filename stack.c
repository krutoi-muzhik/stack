#ifndef STACK_H_
#define STACK_H_
#include "stack.h"
#endif

const data_t poison = 0xBEBABABA;
const canary_t canary = 0xBEFEDABA;
const char *logs_path = "logs.txt";

unsigned long long
GetCRC (char* data, size_t start, size_t end) {
	const unsigned int Polynomial = 0xEDB88320;
	unsigned long long crc = 0;
	size_t length = end;
	while (length-- > start) {
		crc ^= *data++;
		for (size_t j = 0; j < 8; j++)
			crc = (crc >> 1) ^ (-(int)(crc & 1) & Polynomial);
	}

	return crc;
}

unsigned long long
GetDataHash (stack* stk) {
	unsigned long long hash = 0;
	hash = GetCRC ((char*)stk->data, sizeof (canary_t), sizeof (data_t) * stk->size); // ToDo: save hash on stack
	return hash;
}

unsigned long long
GetStackHash (stack* stk) {
	unsigned long long hash = 0;
	hash = GetCRC ((char*) stk, sizeof (stk->canary1), sizeof (*stk) - sizeof (stk->canary2) - sizeof (stk->hash));
	//hash = GetCRC ((char*) stk, sizeof (stk->canary1), sizeof (stk) - sizeof (stk->canary2) - sizeof (stk->hash));
	return hash;
}

void
StackConstruct (stack *stk) { // ToDo: check is created
	stk->canary1 = canary;
	stk->canary2 = canary;
	stk->size = 0;
	stk->capacity = 1;
	stk->data = (char *) calloc (stk->capacity * sizeof (data_t) + 2 * sizeof (canary_t) + sizeof (unsigned long long), sizeof (char));
	wmemset ((data_t *) (stk->data + sizeof (canary_t)), poison, stk->capacity);
	*((canary_t *) stk->data) = canary;
	*((unsigned long long *) (stk->data + sizeof (canary_t) + stk->capacity * sizeof (data_t))) = GetDataHash (stk);
	*((canary_t *) (stk->data + sizeof (canary_t) + sizeof (data_t) * stk->capacity + sizeof (unsigned long long))) = canary;
	stk->data += sizeof (canary_t);
	stk->hash = GetStackHash (stk);
	return;
}

void
StackDestruct (stack *stk) {
	int error = StackError (stk);
	if (error)
		Dump (stk, error, __FUNCTION__, __LINE__); // ToDo: return???
	
	stk->data -= sizeof (canary_t);
	stk->capacity = 0;
	stk->size = 0;
	stk->hash = 0;
	free (stk->data);
}

void
StackSizeUp (stack *stk) {
	stk->data -= sizeof (canary_t);
	stk->capacity *= 2; // ToDo: 2 - const
	stk->data = (char *) realloc (stk->data, stk->capacity * sizeof (data_t) + 2 * sizeof (canary_t) + 1 * sizeof (unsigned long long));
	wmemset ((data_t *) (stk->data + sizeof (canary_t) + stk->size * sizeof (data_t)), poison, stk->capacity - stk->size);
	*((canary_t *) (stk->data + sizeof (canary_t) + stk->capacity * sizeof (data_t) + sizeof (unsigned long long))) = canary;
	stk->data += sizeof (canary_t);
}

void
StackSizedDown (stack *stk) {
	stk->data -= sizeof (canary_t);
	stk->capacity /= 2;
	stk->data = (char *) realloc (stk->data, stk->capacity * sizeof (data_t) + 2 * sizeof (canary_t) + 1 * sizeof (unsigned long long));
	*((canary_t *) (stk->data + sizeof (canary_t) + stk->capacity * sizeof (data_t) + sizeof (unsigned long long))) = canary;
	stk->data += sizeof (canary_t);
}

void
StackPush (stack *stk, data_t value) {
	int error = StackError (stk);
	if (error)
		Dump (stk, error, __FUNCTION__, __LINE__);

	if (stk->size == stk->capacity)
		StackSizeUp (stk);
	*((data_t *) (stk->data + stk->size * sizeof (data_t))) = value;
	stk->size ++;
	*((unsigned long long *) (stk->data + stk->capacity * sizeof (data_t))) = GetDataHash (stk);
	stk->hash = GetStackHash (stk);
}

data_t
StackPop (stack *stk) {
	int error = StackError (stk);
	if (error)
		Dump (stk, error, __FUNCTION__, __LINE__);

	if (stk->size == stk->capacity / 3)
		StackSizedDown (stk);
	data_t value = 0;
	stk->size--;
	value = *((data_t *) (stk->data + stk->size * sizeof (data_t)));
	*((data_t *) (stk->data + stk->size * sizeof (data_t))) = poison;
	*((unsigned long long *) (stk->data + stk->capacity * sizeof (data_t))) = GetDataHash (stk);
	stk->hash = GetStackHash (stk);
	return value;
}


size_t
GetStackSize (stack *stk) {
	size_t count = 0;
	while ((*((data_t *) (stk->data + count * sizeof (data_t))) != poison) && (count < stk->capacity)) {

		count++;
	}
	return count;
}

size_t
GetStackCapacity (stack *stk) {
	size_t count = 0;
	while (*((canary_t *) (stk->data + count)) != canary)
		count++;
	return (count - sizeof (unsigned long long)) / sizeof (data_t);
}


int
StackError (stack *stk) {
	if (stk == NULL)
		return NULL_POINTER;
	if ((stk->canary1 != canary) || (stk->canary2 != canary))
		return CANARY_KILLED;
	if (((*(canary_t *) (stk->data - sizeof (canary_t))) != canary) || (*((canary_t *) (stk->data + stk->capacity * sizeof (data_t) + sizeof (unsigned long long))) != canary))
		return DATA_INVASION;
	if (stk->size != GetStackSize (stk))
		return SIZE_ERROR;
	if (stk->capacity != GetStackCapacity (stk))
		return CAPACITY_ERROR;
	if (stk->hash != GetStackHash (stk))
		return STACK_HASH_ERROR;
	if (*((unsigned long long *) (stk->data + stk->capacity * sizeof (data_t))) != GetDataHash (stk))
		return DATA_HASH_ERROR;
	return NO_ERRORS;
}

void
PrintStackLogs (stack *stk) {
	FILE *logs_file = fopen (logs_path, "a");

	fprintf (logs_file, "\nStack [%p] {  \n", stk); // ToDo: %p  // ToDo: delete fprintf, "abc" "efg" -> "abcdefg"
	fprintf (logs_file, "	canary1 = %d \n", stk->canary1);
	fprintf (logs_file, "	capacity = %ld \n", stk->capacity);
	fprintf (logs_file, "	size = %ld     \n", stk->size);
	fprintf (logs_file, "	data [%p] {  \n", stk->data);
	fprintf (logs_file, "		data canary1 = %d\n", *((canary_t *) (stk->data - sizeof (canary_t))));
	if (stk->capacity > 0) {
		for (size_t i = 0; i < stk->capacity; i++) {
			// if (isnan (*((data_t *) (stk->data + sizeof (data_t) * i))))
			// 	fprintf (logs_file, "		data[%ld] = NAN\n", i);
			fprintf (logs_file, "		data[%ld] = %d", i, *((data_t *) (stk->data + i * sizeof (data_t))));
			if (*((data_t *) (stk->data + sizeof (data_t) * i)) == poison)
				fprintf (logs_file, "(poison)");
			fprintf (logs_file, "\n");
		}
	}
	fprintf (logs_file, "		data hash = %lld\n", *((unsigned long long *) (stk->data + stk->capacity * sizeof (data_t))));
	fprintf (logs_file, "		data canary2 = %d\n", *((canary_t *) (stk->data + stk->capacity * sizeof (data_t) + sizeof (unsigned long long))));
	fprintf (logs_file, "	hash = %lld    \n", stk->hash);
	fprintf (logs_file, "	canary2 = %d   \n", stk->canary2);
	fprintf (logs_file, "}\n\n");
	fclose (logs_file);
}

void
PrintErrorLogs (const char *error, const char *function, const int line, const int error_number) {
	if (error != "NORMALIN NORMALIN") // ToDo: strcmp
		printf ("ERROR!!! [%d] %s check logs\n", error_number, error);
	FILE *logs_file = fopen (logs_path, "a");
	char *current_time = Timenow ();

	fprintf (logs_file, "%s\n Stack [%d] %s in function %s on line %d\n", current_time, error_number, error, function, line);
	fclose (logs_file);
}

char *
Timenow () {
	time_t t = time (NULL);
	return asctime (localtime (&t));
}

void
Dump (stack *stk, int error, const char *function, const int line) {
	switch (error) {
		case NO_ERRORS:
			PrintErrorLogs ("NORMALIN NORMALIN", function, line, error);
			PrintStackLogs (stk);
			break;

		case NULL_POINTER:
			PrintErrorLogs ("NULL POINTER IS GIVEN", function, line, error);
			PrintStackLogs (stk);
			break;
			// ToDo: empty lines
		case CANARY_KILLED:
			PrintErrorLogs ("CANARY KILLED MURDERED DEAD", function, line, error);
			PrintStackLogs (stk);
			break;
		case DATA_INVASION:
			PrintErrorLogs ("DATA IMPOSTOR DETECTED", function, line, error);
			PrintStackLogs (stk);
			break;
		case SIZE_ERROR:
			PrintErrorLogs ("INCORRECT SIZE", function, line, error);
			PrintStackLogs (stk);
			break;
		case CAPACITY_ERROR:
			PrintErrorLogs ("INCORRECT CAPACITY", function, line, error);
			PrintStackLogs (stk);
			break;
		case STACK_HASH_ERROR:
			PrintErrorLogs ("INCORRECT COUNT OF STACK HASH", function, line, error);
			PrintStackLogs (stk);
			break;
		case DATA_HASH_ERROR:
			PrintErrorLogs ("INCORRECT COUNT OF DATA HASH", function, line, error);
			PrintStackLogs (stk);
			break;
		case STACK_EMPTE:
			PrintErrorLogs ("empty stack", function, line, error);
			PrintStackLogs (stk);
			break;
		default:
			PrintErrorLogs ("UNKNOWN ERROR", function, line, error);
			PrintStackLogs (stk);
			break;
	}
}
