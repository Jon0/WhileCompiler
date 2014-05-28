/*
 * X86Register.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include "X86Register.h"

namespace std {

X86Register::X86Register(string n) {
	name = n;
}

X86Register::~X86Register() {}

string X86Register::place() {
	return "%"+name;
}

} /* namespace std */
