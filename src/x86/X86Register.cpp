/*
 * X86Register.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include "X86Reference.h"
#include "X86Register.h"

namespace std {

X86Register::X86Register(string n) {
	name = n;
}

X86Register::~X86Register() {}

string X86Register::place() {
	return "%"+sizeDesc()+name;
}

void X86Register::assign(shared_ptr<X86Reference> r) {
	ref = r;
}

string X86Register::sizeDesc() {
	return "e";
}

} /* namespace std */
