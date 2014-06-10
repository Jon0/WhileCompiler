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
	next_id = 0;
	is_mmx = n[0] == 'x'; // for xmm registers
}

X86Register::~X86Register() {}

string X86Register::getName() {
	return name;
}

bool X86Register::isMmx() {
	return is_mmx;
}

bool X86Register::inUse() {
	return (use_id.size() > 0);
}

void X86Register::free(int id) {
	//cout << "free " << name << endl;
	use_id.erase(id);
}

int X86Register::use() {
	//cout << "assigning register " << name << endl;
	int id = next_id++;
	use_id.insert(id);
	return id;
}

int X86Register::getSize() {
	return current_size;
}

void X86Register::setSize(int i) {
	current_size = i;
}


string X86Register::place() {
	return place( current_size );
}

string X86Register::place(int w) {
	string out = "%";
	if (!is_mmx && w == 4) out += "e";
	else if (!is_mmx && w >= 8) out += "r";
	out += name;

	return out;
}

void X86Register::assign(shared_ptr<X86Reference> i) {
	current_size = i->typeSize();
	program->addInstruction( "text", make_shared<InstrMov>( check(i), ref() ) );
}

void X86Register::assignAddrOf(shared_ptr<X86RegAddrRef> r) {
	current_size = 8;
	program->addInstruction( "text", make_shared<InstrLea>( r, ref() ) );
}

void X86Register::add( shared_ptr<X86Reference> i ) {
	program->addInstruction( "text", make_shared<InstrAdd>( check(i), ref() ) );
}

void X86Register::sub( shared_ptr<X86Reference> i ) {
	program->addInstruction( "text", make_shared<InstrSub>( check(i), ref() ) );
}

void X86Register::multiply( shared_ptr<X86Reference> i ) {
	program->addInstruction( "text", make_shared<InstrMul>( check(i), ref() ) );
}

void X86Register::divide( shared_ptr<X86Reference> i ) {
	assign( program->intdivide(ref(), i, false) );
}

void X86Register::mod( shared_ptr<X86Reference> i ) {
	assign( program->intdivide(ref(), i, true) );
}

void X86Register::andBitwise( shared_ptr<X86Reference> i ) {
	program->addInstruction( "text", make_shared<InstrAnd>( check(i)->place(), place() ) );
}

void X86Register::orBitwise( shared_ptr<X86Reference> i ) {
	program->addInstruction( "text", make_shared<InstrOr>( check(i)->place(), place() ) );
}

void X86Register::compare( shared_ptr<X86Reference> i ) {
	program->addInstruction( "text", make_shared<InstrCmp>( check(i)->place(), place() ) ); // sets flags
}

void X86Register::setFromFlags(string type) {
	string bytename = "%"+name.substr(0,1)+"l";
	program->addInstruction( "text", make_shared<InstrSet>( type, bytename ) );
	program->addInstruction( "text", make_shared<InstrMov>( "zb", make_shared<X86LabeledRef>(bytename), ref() ) );
}

shared_ptr<X86RegRef> X86Register::ref() {
	return make_shared<X86RegRef>( shared_from_this(), current_size );
}

shared_ptr<X86RegAddrRef> X86Register::ref(int i) {
	return make_shared<X86RegAddrRef>( shared_from_this(), i, current_size );
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

shared_ptr<X86Reference> X86Register::check( shared_ptr<X86Reference> i ) {
	shared_ptr<X86Reference> source = i;
	if (is_mmx != i->isMmx()) {
		source = i->save(program);
	}
	return source;
}

} /* namespace std */
