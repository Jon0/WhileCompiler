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
	placement.begin = -1;
	constant = s;
	isOnStack = false;
	type_size = 4;
}

X86Reference::X86Reference(shared_ptr<X86Register> base, StackSpace ss) {
	stackBase = base;
	placement = ss;
	isOnStack = true;
	reg = NULL;
	type_size = ss.size;
}

X86Reference::X86Reference(shared_ptr<X86Register> r, int ts) {
	// reference content of register too
	placement.begin = r->getRefStackOffset();
	isOnStack = (placement.begin >= 0);
	reg = r;
	type_size = ts;

	if (type_size > 8) cout << "register size " << type_size << " invalid" << endl;
}

X86Reference::~X86Reference() {
	// TODO Auto-generated destructor stub
}

bool X86Reference::isPointer() {
	return (type_size > 8);
}

int X86Reference::typeSize() {
	return type_size;
}

int X86Reference::stackOffset() {
	return placement.begin;
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
		return to_string(placement.begin)+"(%rbp)";
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
	return make_shared<X86Reference>( stackBase, StackSpace{ placement.begin + b, s, placement.type } );
}

/*
 * update stack/heap value
 */
// TODO shared_ptr<X86Program> paramater
shared_ptr<X86Instruction> X86Reference::setValue( shared_ptr<X86Reference> r) {
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

/*
 * pointer to this object into a register
 */
void X86Reference::assignRegisterPointer(shared_ptr<X86Program> p, shared_ptr<X86Register> r) {
	p->addInstruction( "text", make_shared<InstrMov>(  make_shared<X86Reference>(stackBase, 8), make_shared<X86Reference>(r, 8) ) );
	p->addInstruction( "text", make_shared<InstrAdd>( "$"+to_string(placement.begin), r->place(8) ) ); // TODO register is no longer a reference
}

} /* namespace std */
