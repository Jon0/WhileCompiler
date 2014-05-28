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
