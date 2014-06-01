/*
 * X86Reference.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "X86Reference.h"

namespace std {

X86Reference::X86Reference(string s) {
	constant = s;
	reg = NULL;
	offset = 0;
	type_size = 8;
	use_addr = false;
	is_live = true;
}

X86Reference::X86Reference(long s) {
	constant = "$"+to_string(s);
	reg = NULL;
	offset = 0;
	type_size = 8;
	use_addr = false;
	is_live = true;
}

X86Reference::X86Reference(shared_ptr<X86Register> base, int ts) {
	reg = base;
	offset = 0;
	type_size = ts;
	use_addr = false;
	is_live = true;
}

X86Reference::X86Reference(shared_ptr<X86Register> base, int o, int ts) {
	reg = base;
	offset = o;
	type_size = ts;
	use_addr = true;
	is_live = true;
}

X86Reference::X86Reference(shared_ptr<X86Register> base, StackSpace ss):
		X86Reference(base, ss.begin, ss.size) {}

X86Reference::~X86Reference() {
	// TODO Auto-generated destructor stub
}

bool X86Reference::isLive() {
	return is_live;
}

void X86Reference::free() {
	if (reg) {
		reg->free();
		is_live = false;
	}
}

shared_ptr<X86Reference> X86Reference::index(int b, int s) {
	return make_shared<X86Reference>( reg, offset + b, s );
}

int X86Reference::typeSize() {
	return type_size;
}

int X86Reference::getOffset() {
	return offset;
}

string X86Reference::place() {
	return place(type_size);
}

string X86Reference::place(int w) {
	if (!is_live) {
		cout << "error -- using dead reference" << endl;
	}

	if (reg && use_addr) {
		return to_string(offset)+"("+reg->place(w)+")";
	}
	else if (reg) {
		return reg->place(w);
	}
	else {
		return constant;
	}
}

bool X86Reference::isRegister() {
	return reg && !use_addr; // a non addressed register
}

shared_ptr<X86Register> X86Reference::getRegister() {
	return reg;
}

/*
 * update stack/heap value -this function no longer used
 */
// TODO shared_ptr<X86Program> paramater
shared_ptr<X86Instruction> X86Reference::setValue( shared_ptr<X86Reference> r) {
	cout << "X86Reference::setValue should no longer be used" << endl;

	// the register will no longer match
	if (reg) {
		reg->assign( NULL );
		reg = NULL;
	}
	return make_shared<InstrMov>( r, shared_from_this() );
}

} /* namespace std */
