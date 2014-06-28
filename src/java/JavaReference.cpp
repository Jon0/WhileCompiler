/*
 * JavaReference.cpp
 *
 *  Created on: 19/06/2014
 *      Author: asdf
 */

#include "JavaReference.h"

namespace std {

JavaReference::JavaReference() {}

JavaReference::~JavaReference() {}

ConstReference::ConstReference(short s, unsigned char size) {
	addr = s;
	ref_size = size;
}

ConstReference::~ConstReference() {}

unsigned char ConstReference::size() {
	return ref_size;
}

bytecode ConstReference::toByteCode() {
	bytecode b;
	if (ref_size == 1) {
		write_u1(b, addr);
	}
	else {
		write_u2(b, addr);
	}
	return b;
}

} /* namespace std */
