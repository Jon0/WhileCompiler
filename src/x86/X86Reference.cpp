/*
 * X86Reference.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include "X86Reference.h"

namespace std {

X86Reference::X86Reference(string s) {
	stackPlaceOffset = -1;
	constant = s;
	isOnStack = false;
	type_size = 4;
}

X86Reference::X86Reference(StackSpace ss) {
	stackPlaceOffset = ss.begin;
	isOnStack = true;
	reg = NULL;
	type_size = ss.size;
}

X86Reference::X86Reference(shared_ptr<X86Register> r, int ts) {
	// reference content of register too
	stackPlaceOffset = r->getRefStackOffset();
	isOnStack = (stackPlaceOffset >= 0);
	reg = r;
	type_size = ts;
}

X86Reference::~X86Reference() {
	// TODO Auto-generated destructor stub
}

int X86Reference::typeSize() {
	return type_size;
}

int X86Reference::stackOffset() {
	return stackPlaceOffset;
}

string X86Reference::place() {
	if (reg) {
		return reg->place(type_size);
	}
	else if (isOnStack) {
		return stackPlace();
	}
	else {
		return constant;
	}
}

string X86Reference::place(int w) {
	if (reg) {
		return reg->place(w);
	}
	else {
		return place();
	}
}

string X86Reference::stackPlace() {
	if (isOnStack) {
		return to_string(stackPlaceOffset)+"(%rbp)";
	}
	else if (constant.length() > 0) {
		return constant;
	}
	else if (reg) {
		return reg->place();
	}
	else {
		return "error";
	}
}


shared_ptr<X86Reference> X86Reference::index(int b, int s) {
	return make_shared<X86Reference>( StackSpace{ stackPlaceOffset + b, s, NULL } );
}

/*
 * update stack/heap value
 */
shared_ptr<X86Instruction> X86Reference::setValue(shared_ptr<X86Reference> r) {
	// the register will no longer match
	if (reg) {
		reg->assign( NULL );
		reg = NULL;
	}
	return make_shared<InstrMov>( r, make_shared<X86Reference>(stackPlace()) );
}

shared_ptr<X86Instruction> X86Reference::assignRegister(shared_ptr<X86Register> r) {
	reg = r;
	reg->assign( shared_from_this() );

	if (reg) {
		return make_shared<InstrMov>( make_shared<X86Reference>(stackPlace()), make_shared<X86Reference>(reg, type_size) );
	}
	else {
		// TODO !!! -- empty instruction?
		return make_shared<InstrSkip>();
	}

}

} /* namespace std */
