#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 0 - void
// 1 - null
// 2 - bool
// 3 - char
// 4 - int
// 5 - float
// 7- string
// 8 - list
// 16 - record

void *makestr(const char *text) {
	long length = strlen(text), i;
	long *list = malloc( (length+1) * 16 );
	list[0] = 4; // is int
	list[1] = length;
	for (i = 0; i < length; ++i) {
		list[(i+1)*2] = 3; // char
		list[(i+1)*2 + 1] = text[i];
	}


	long *obj = malloc(16);
	obj[0] = 7; // is string
	obj[1] = (long) list;
	return obj;
}


const char *strobj(void *in) {
	char *buf = malloc(256);
	long *lp = in;
	long *location;
	const char *str_location;
	double d_value;
	long arg;
	long i;

	switch (lp[0]) {
	case 1:
		sprintf(buf, "null");
		break;
	case 2:
		if (lp[1]) sprintf(buf, "true");
		else sprintf(buf, "false");
		break;
	case 3:
		sprintf(buf, "%c", (char)lp[1]);
		break;
	case 4:
		sprintf(buf, "%ld", lp[1]);
		break;
	case 5:
		memcpy(&d_value, &lp[1], sizeof(double));
		char fltbuf [64];
		str_location = (const char *)&fltbuf;
		sprintf(fltbuf, "%g", d_value);
		if (!(strstr(str_location, ".") || strstr(str_location, "e"))) {
			strcat(&fltbuf[0], ".0");
		}
		sprintf(buf, "%s", fltbuf);
		break;
	case 7:
		location = (long *)lp[1];
		for (i = 0; i < location[1]; ++i) {
			strcat(buf, strobj( &location[(i+1)*2] ));
		}
		break;
	case 8:
		location = (long *)lp[1];
		sprintf(buf, "[");
		for (i = 0; i < location[1]; ++i) {
			strcat(buf, strobj( &location[(i+1)*2] ));
			if (i < location[1]-1) strcat(buf, ", ");
		}
		strcat(buf, "]");
		break;
	case 16:
		sprintf(buf, "{}");
		break;
	default:
		sprintf(buf, "t%ld?", lp[0]);
		break;
	}
	return buf;
}

void print(void *f) {
	//printf("print is called with arg %p\r\n", f);
	printf("%s\n", strobj(f));
}

long equiv(void *a, void *b) {
	long *la = a;
	long *lb = b;
	int i, offset;
	long *inla, *inlb;

	// different types
	if (la[0] != lb[0]) {
		return 0;
	}

	long type = la[0];
	switch (type) {
	case 1:
		return 1;
	case 2:
	case 3:
	case 4:
	case 5:
		return la[1] == lb[1];
	case 7:
	case 8:
		inla = (long *)la[1];
		inlb = (long *)lb[1];

		// check lengths
		if (!equiv(inla, inlb)) return 0;

		// check elements
		for (i = 0; i < inla[1]; ++i) {
			offset = (i+1)*2;
			if ( !equiv( &inla[offset], &inlb[offset] ) ) return 0;
		}
		return 1;
	case 16:
		return 1;
	default:
		return 0;
	}



	return la[1] == lb[1]; // TODO pointer types
}

void *clone(void *a) {
	long *la = a;
	int i, offset;
	long *inla;
	long *newspace, *innerspace, *vv;

	long type = la[0];
	switch (type) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		newspace = malloc(16);
		newspace[0] = la[0];
		newspace[1] = la[1];
		return newspace;
	case 7:
	case 8:
		inla = (long *)la[1];
		innerspace = malloc( (inla[1] + 1) * 16 );

		// clone length and  elements
		for (i = 0; i < inla[1] + 1; ++i) {
			offset = i*2;
			vv = clone(&inla[offset]);
			innerspace[offset] = vv[0];
			innerspace[offset+1] = vv[1];
		}

		newspace = malloc(16);
		newspace[0] = la[0];
		newspace[1] = (long) innerspace;
		return newspace;
	case 16:
		return a;
	default:
		return a;
	}




	return a;
}

void *append(void *a, void *b) {
	long *obj_a = a;
	long *alocation = (long *)obj_a[1]; // pointer to object content
	long alength = alocation[1];		// length of a

	long *obj_b = b;

	// append in string mode
	if (obj_a[0] == 7 && obj_b[0] == 8) {
		obj_b = makestr( strobj( obj_b ) );
	}

	// append non list type
	int copy_single = !(obj_b[0] == 7 || obj_b[0] == 8);
	long *blocation = (long *)obj_b[1];
	long blength;
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
