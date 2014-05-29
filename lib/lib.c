#include <stdio.h>

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

void printL(int *in) {
	int i;
	printf("[");
	int length = in[0];
	for (i = 0; i < length; ++i) {
		printf("%d", in[i+1]);
		if (i < length-1) {
			printf(", ");
		}
	}
	printf("]\r\n");
}
