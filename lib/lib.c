#include <stdio.h>

// 8 is arrays
// 4 is ints

void print(const char *in) {
	printf("%s\r\n", in);
}

void printB(int in) {
	if (in) {
		printf("true\r\n");
	}
	else {
		printf("false\r\n");
	}
}

void printI(int in) {
	printf("%d\r\n", in);
}

void printL(long *location) {
	printf("addr = %p\r\n", location);
	printf("addr[0] = %ld\r\n", location[0]);

	// arg is pointer to list
	long *in = (long *)location[1];
	printf("list length = %ld\r\n", in[0]);

	long i;
	printf("[");
	long length = in[0];
	for (i = 0; i < length; ++i) {
		printf("%ld", in[i+1]);
		if (i < length-1) {
			printf(", ");
		}
	}
	printf("]\r\n");
}
