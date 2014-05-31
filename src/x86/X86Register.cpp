/*
 * X86Register.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include "X86Reference.h"
#include "X86Register.h"

#include <iostream>

namespace std {

X86Register::X86Register(shared_ptr<X86Program> p, string n) {
	program = p;
	name = n;
	current_size = 8;
	in_use = false;
}

X86Register::~X86Register() {}

string X86Register::getName() {
	return name;
}

bool X86Register::inUse() {
	return in_use;
}

void X86Register::free() {
	//cout << "free " << name << endl;
	in_use = false;
}

void X86Register::setAsUsed() {
	//cout << "assigning register " << r->getName() << endl;
	in_use = true;
}

int X86Register::getSize() {
	return current_size;
}

void X86Register::setSize(int i) {
	current_size = i;
}


string X86Register::place() {
	return "%"+sizeDesc()+name;
}

string X86Register::place(int w) {
	string size = "";
	if (w == 4) size = "e";
	else if (w >= 8) size = "r";

	return "%"+size+name;
}

void X86Register::assign(shared_ptr<X86Reference> r) {
	setAsUsed();
	current_size = r->typeSize();
	program->addInstruction( "text", make_shared<InstrMov>( r, ref() ) );

}

void X86Register::add( shared_ptr<X86Reference> i ) {
	program->addInstruction( "text", make_shared<InstrAdd>( i->place(), place() ) );
}

void X86Register::multiply( shared_ptr<X86Reference> i ) {
	program->addInstruction( "text", make_shared<InstrMul>( i->place(), place() ) );
}

void X86Register::compare( shared_ptr<X86Reference> i ) {
	program->addInstruction( "text", make_shared<InstrCmp>( i->place(), place() ) ); // sets flags
}

void X86Register::setFromFlags(string type) {
	string bytename = "%"+name.substr(0,1)+"l";
	program->addInstruction( "text", make_shared<InstrSet>( type, bytename ) );
	program->addInstruction( "text", make_shared<InstrMov>( "zbl", bytename, place() ) );
}


shared_ptr<X86Reference> X86Register::ref() {
	return make_shared<X86Reference>( shared_from_this(), current_size );
}

///*
// * pointer to this object into a register
// */
//void X86Reference::assignRegisterPointer(shared_ptr<X86Program> p, shared_ptr<X86Register> r) {
//	p->addInstruction( "text", make_shared<InstrMov>(  make_shared<X86Reference>(stackBase, 8), make_shared<X86Reference>(r, 8) ) );
//	p->addInstruction( "text", make_shared<InstrAdd>( "$"+to_string(placement.begin), r->place(8) ) ); // TODO register is no longer a reference
//}

// probably no longer used
int X86Register::getRefStackOffset() {
//	if (ref) {
//		return ref->stackOffset();
//	}
//	else {
//		return -1;
//	}
	return -1;
}

// TODO use current_size to determine label
string X86Register::sizeDesc() {
	if (current_size == 8) {
		return "r";
	}
	else {
		return "e";
	}
}

} /* namespace std */
