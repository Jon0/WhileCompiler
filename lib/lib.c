#include <stdio.h>

// 8 is arrays
// 4 is ints

void print_v(void *in) {
	long *lp = in;
	long *location;
	long arg;
	long i;

	switch (lp[0]) {
	case 4:
		printf("%ld", lp[1]);
		break;
	case 8:
		location = (long *)lp[1];
		printf("[length %ld]", location[1]);
		printf("[");
		for (i = 0; i < location[1]; ++i) {
			print_v( &location[(i+1)*2] );
			if (i < location[1]-1) printf(", ");
		}
		printf("]");
		break;
	default:
		printf("?");
		break;
	}
}

void print(void *f) {
	printf("print is called with arg %p\r\n", f);

	print_v(f);
	printf("\r\n");
}
