/*
 * X86Reference.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include <cstring>
#include <iostream>

#include "X86Reference.h"

namespace std {

X86Reference::X86Reference(int ts) {
	type_size = ts;
}

X86Reference::~X86Reference() {}

string X86Reference::place() {
	return place(type_size);
}

int X86Reference::typeSize() {
	return type_size;
}

X86LabeledRef::X86LabeledRef(string s): X86Reference(8) {
	constant = s;
}

X86LabeledRef::~X86LabeledRef() {}

string X86LabeledRef::place(int) {
	return constant;
}

X86ConstRef::X86ConstRef(long s): X86Reference(8) {
	constant = s;
}

X86ConstRef::~X86ConstRef() {}

string X86ConstRef::place(int) {
	return "$"+to_string(constant);
}

X86RealRef::X86RealRef(double s): X86Reference(8) {
	memcpy(&constant, &s, sizeof(long));
}

X86RealRef::~X86RealRef() {}

string X86RealRef::place(int) {
	return "$"+to_string(constant);
}


X86RegRef::X86RegRef(shared_ptr<X86Register> base, int ts):
		X86Reference(ts) {
	reg = base;
	reg_link_id = reg->use();
	is_live = true;
}

X86RegRef::~X86RegRef() {
	free();
}

bool X86RegRef::isLive() {
	return is_live;
}

void X86RegRef::free() {
	if (reg) {
		reg->free(reg_link_id);
		is_live = false;
	}
}

string X86RegRef::place(int w) {
	if (!is_live) {
		cout << "error -- using dead reference" << endl;
	}

	return reg->place(w);
}

shared_ptr<X86RegAddrRef> X86RegRef::index(int o, int newsize) {
	return make_shared<X86RegAddrRef>( reg, o, newsize );
}

shared_ptr<X86RegAddrRef> X86RegRef::index(int o) {
	return index( o, typeSize() );
}

shared_ptr<X86Register> X86RegRef::getRegister() {
	return reg;
}

X86RegAddrRef::X86RegAddrRef(shared_ptr<X86Register> base, int o, int ts):
		X86Reference(ts) {
	reg = base;
	offset = o;
	reg_link_id = reg->use();
	is_live = true;
}

X86RegAddrRef::~X86RegAddrRef() {
	free();
}

bool X86RegAddrRef::isLive() {
	return is_live;
}

void X86RegAddrRef::free() {
	if (reg) {
		reg->free(reg_link_id);
		is_live = false;
	}
}

string X86RegAddrRef::place(int w) {
	if (!is_live) {
		cout << "error -- using dead reference" << endl;
	}

	string out = "";
	if (offset) out += to_string(offset);
	out += "("+reg->place(w)+")";

	return out;
}

shared_ptr<X86RegAddrRef> X86RegAddrRef::index(int o, int newsize) {
	return make_shared<X86RegAddrRef>( reg, offset + o, newsize );
}

shared_ptr<X86RegAddrRef> X86RegAddrRef::index(int o) {
	return index( o, typeSize() );
}

int X86RegAddrRef::getOffset() {
	return offset;
}

shared_ptr<X86Register> X86RegAddrRef::getRegister() {
	return reg;
}

} /* namespace std */
