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

string X86Register::place(int w) {
	string size = "";
	if (w == 4) size = "e";
	else if (w == 8) size = "r";

	return "%"+size+name;
}

void X86Register::assign(shared_ptr<X86Reference> r) {
	ref = r;
}

int X86Register::getRefStackOffset() {
	if (ref) {
		return ref->stackOffset();
	}
	else {
		return -1;
	}
}

string X86Register::sizeDesc() {
	return "e";
}

} /* namespace std */
