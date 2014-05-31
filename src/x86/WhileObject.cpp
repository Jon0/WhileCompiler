/*
 * WhileObject.cpp
 *
 *  Created on: 30/05/2014
 *      Author: remnanjona
 */

#include "../lang/Type.h"
#include "WhileObject.h"

#include "X86Program.h"
#include "X86Reference.h"
#include "X86Register.h"

namespace std {

WhileObject::WhileObject( shared_ptr<X86Program> p ) {
	program = p;
	base = program->getBPRegister();
	space.begin = 0;
	space.size = 0;
	type = 0;
}

WhileObject::~WhileObject() {}

void WhileObject::putOnStack() {
	space = program->allocateStack(16);
}

void WhileObject::pushStack() {
	program->addInstruction( "text", make_shared<InstrPush>( valueRef() ) );
	program->addInstruction( "text", make_shared<InstrPush>( tagRef() ) );
}

void WhileObject::setLocation( shared_ptr<X86Register> r ) {
	base = r;
}

void WhileObject::setLocation( shared_ptr<X86Register> r, StackSpace s ) {
	base = r;
	space = s;
}

void WhileObject::initialise( shared_ptr<X86Reference> v, int t, bool write ) {
	ref = v;
	type = make_shared<X86Reference>(t);
	if ( write ) {
		writeMem();
	}
}

void WhileObject::initialise( shared_ptr<X86Reference> v, bool write ) {
	initialise( v, 4, write );
}

void WhileObject::assign( shared_ptr<WhileObject> other, bool write ) {
	ref = other->valueDirect();
	type = other->tagDirect();

	if ( write ) {
		writeMem();
	}
}

void WhileObject::free() {
	base->free();
}

void WhileObject::writeMem() {
	if (!ref) {
		cout << "error writing memory" << endl;
		return;
	}

	program->addInstruction( "text", make_shared<InstrMov>( type, tagRef() ) );
	program->addInstruction( "text", make_shared<InstrMov>( ref, valueRef() ) );
}

shared_ptr<X86Reference> WhileObject::tagDirect() {
	return type;
}

shared_ptr<X86Reference> WhileObject::valueDirect() {
	return ref;
}

// function to put address in register?
// or return address as reference then register.assign(addr)
shared_ptr<X86Reference> WhileObject::addrRef() {
	shared_ptr<X86Register> r = program->getFreeRegister();
	r->assign( base->ref() );
	r->add( make_shared<X86Reference>(space.begin) );

	return make_shared<X86Reference>( r, 8 );
}

shared_ptr<X86Reference> WhileObject::tagRef() {
	return make_shared<X86Reference>( base, space.begin, 8 );
}

shared_ptr<X86Reference> WhileObject::valueRef() {
	return make_shared<X86Reference>( base, space.begin + 8, 8 );
}

WhileList::WhileList( shared_ptr<X86Program> p ): WhileObject(p) {}
WhileList::~WhileList() {}

void WhileList::initialise(shared_ptr<X86Reference> v, bool write) {
	shared_ptr<X86Register> r1 = program->getFreeRegister();
	r1->assign( v );
	r1->multiply( make_shared<X86Reference>(16) );

	shared_ptr<X86Register> r2 = program->malloc( r1->ref() );
	program->addInstruction( "text", make_shared<InstrMov>( make_shared<X86Reference>(8), tagRef() ) );
	program->addInstruction( "text", make_shared<InstrMov>( r2->ref(), valueRef() ) );
	r2->free();
	r1->free();

	shared_ptr<WhileObject> l = make_shared<WhileObject>(program);
	length(l);
	program->addInstruction( "text", make_shared<InstrMov>( make_shared<X86Reference>(4), l->tagRef() ) );
	program->addInstruction( "text", make_shared<InstrMov>( v, l->valueRef() ) );
	l->free();
}

void WhileList::length(shared_ptr<WhileObject> inout) {
	shared_ptr<X86Register> r = program->getFreeRegister();
	r->assign( valueRef() );
	inout->setLocation(r);
	//return make_shared<WhileObject>(program, r, 4);
}

void WhileList::get(shared_ptr<WhileObject> inout, shared_ptr<X86Reference> ref) {
	shared_ptr<X86Register> r = program->getFreeRegister();
	r->assign( ref );
	r->add( make_shared<X86Reference>(1) );
	r->multiply( make_shared<X86Reference>(16) );
	r->setSize(8);
	r->add( valueRef() );
	inout->setLocation(r);
	//return make_shared<WhileObject>(program, r, inner_type);
}

} /* namespace std */
