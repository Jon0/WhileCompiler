#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 1 - null
// 2 - bool
// 3 - char
// 4 - int
// 5 - float
// 7- string
// 8 - list
// 16 - record


void print_v(void *in) {
	long *lp = in;
	long *location;
	const char *str_location;
	double d_value;
	long arg;
	long i;

	switch (lp[0]) {
	case 1:
		printf("null");
		break;
	case 2:
		if (lp[1]) printf("true");
		else printf("false");
		break;
	case 3:
		printf("%c", (char)lp[1]);
		break;
	case 4:
		printf("%ld", lp[1]);
		break;
	case 5:
		memcpy(&d_value, &lp[1], sizeof(double));
		printf("%f", d_value);
		break;
	case 7:
		location = (long *)lp[1];
		//printf("[str length %ld]", location[1]);
		for (i = 0; i < location[1]; ++i) {
			print_v( &location[(i+1)*2] );
		}
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
	//printf("print is called with arg %p\r\n", f);
	print_v(f);
	printf("\r\n");
}

int equiv(void *a, void *b) {
	//printf("equiv is called with arg %p, %p\r\n", a, b);
	long *la = a;
	long *lb = b;

	//printf("comparing %ld == %ld, get %d\n", la[1], lb[1], (la[1] == lb[1]));

	if (la[0] != lb[0]) {
		//printf("comparing types differ");
		return 0; // different types
	}
	return la[1] == lb[1]; // TODO pointer types
}

void *append(void *a, void *b) {
	long *obj_a = a;
	long *alocation = (long *)obj_a[1];
	long alength = alocation[1];

	long *obj_b = b;
	long *blocation = (long *)obj_b[1];
	long blength;

	int copy_single = !(obj_b[0] == 7 || obj_b[0] == 8);

	if (copy_single) {
		blength = 1;
	}
	else {
		blength = blocation[1];
	}

	long newLength = alength + blength;
	long *newlocation = malloc(16+16*newLength);

	// TODO clone nested items
	newlocation[0] = 4;
	newlocation[1] = newLength;
	memcpy(&newlocation[2], &alocation[2], 16*alength);
	if (copy_single) {
		memcpy(&newlocation[2+2*alength], obj_b, 16);
	}
	else {
		memcpy(&newlocation[2+2*alength], &blocation[2], 16*blength);
	}

	long *obj_s = malloc(16);
	obj_s[0] = obj_a[0];
	obj_s[1] = (long)newlocation;

	return obj_s;
}
