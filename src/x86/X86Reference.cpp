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
}

X86Reference::X86Reference(int i) {
	stackPlaceOffset = i;
	isOnStack = true;
	reg = NULL;
}

X86Reference::X86Reference(shared_ptr<X86Register> r) {
	stackPlaceOffset = -1;
	isOnStack = false;
	reg = r;
}

X86Reference::~X86Reference() {
	// TODO Auto-generated destructor stub
}

string X86Reference::place() {
	if (reg) {
		return reg->place();
	}
	else if (isOnStack) {
		return stackPlace();
	}
	else {
		return constant;
	}
}

string X86Reference::stackPlace() {
	return to_string(stackPlaceOffset)+"(%rbp)";
}

shared_ptr<X86Instruction> X86Reference::setValue(string v) {
	return make_shared<InstrMov>( v, stackPlace() );
}

shared_ptr<X86Instruction> X86Reference::assignRegister(shared_ptr<X86Register> r) {
	reg = r;

	if (reg) {
		return make_shared<InstrMov>( stackPlace(), reg->place() );
	}
	else {
		// TODO !!! -- empty instruction?
		return make_shared<InstrSkip>();
	}

}

} /* namespace std */
