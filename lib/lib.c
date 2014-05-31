#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// 8 is arrays
// 4 is ints

void print_v(void *in) {
	long *lp = in;
	long *location;
	const char *str_location;
	long arg;
	long i;

	switch (lp[0]) {
	case 2:
		if (lp[1]) printf("true");
		else printf("false");
		break;
	case 4:
		printf("%ld", lp[1]);
		break;
	case 5:
		//d_location = (double *)lp;
		printf("double");
		break;
	case 7:
		str_location = (const char *)lp[1];
		printf("%s", str_location);
		break;
	case 8:
		location = (long *)lp[1];
		//printf("[length %ld]", location[1]);
		printf("[");
		for (i = 0; i < location[1]; ++i) {
			print_v( &location[(i+1)*2] );
			if (i < location[1]-1) printf(", ");
		}
		printf("]");
		break;
	case 16:
		printf("{}");
		break;
	default:
		printf("t%ld?", lp[0]);
		break;
	}
}

void print(void *f) {
	char buf [100];
	sprintf(buf, "%g", (int)17.9);

	//printf("print is called with arg %p\r\n", f);
	print_v(f);
	printf("\r\n");
}
