/*
 * Bytecode.cpp
 *
 *  Created on: 24/06/2014
 *      Author: asdf
 */

#include "Bytecode.h"

namespace std {

void write_u4(bytecode &bytes, unsigned int i) {
	bytes.push_back( (unsigned char) (i >> 24) );
	bytes.push_back( (unsigned char) (i >> 16) );
	bytes.push_back( (unsigned char) (i >> 8) );
	bytes.push_back( (unsigned char) (i >> 0) );
}

void write_u2(bytecode &bytes, unsigned short i) {
	bytes.push_back( (unsigned char) (i >> 8) );
	bytes.push_back( (unsigned char) (i >> 0) );
}

void write_u1(bytecode &bytes, unsigned char i) {
	bytes.push_back( (unsigned char) i );
}

void write_str(bytecode &bytes, const string i) {
	for (unsigned char c: i) {
		write_u1(bytes, c);
	}
}

void write_list(bytecode &bytes, const bytecode i) {
	for (unsigned char c: i) {
		write_u1(bytes, c);
	}
}

} /* namespace std */
