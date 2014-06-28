/*
 * Bytecode.h
 *
 *  Created on: 24/06/2014
 *      Author: asdf
 */

#ifndef BYTECODE_H_
#define BYTECODE_H_

#include <string>
#include <vector>

namespace std {

typedef vector<unsigned char> bytecode;

void write_u4(bytecode &, unsigned int i);
void write_u2(bytecode &, unsigned short i);
void write_u1(bytecode &, unsigned char i);
void write_str(bytecode &, const string i);
void write_list(bytecode &, const bytecode i);

} /* namespace std */

#endif /* BYTECODE_H_ */
