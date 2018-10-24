#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#define test_0_zero main

enum TCHARS {
	TA,
	TB
};

int test_0_zero(void) {
	void * ptr = NULL;
	bool flag = false;

	char zero = '0';

	printf("sizeof(enum TCHARS) = %zu.\n", sizeof(enum TCHARS));

	return EXIT_SUCCESS;
}