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
	initialised = false;
}

WhileObject::~WhileObject() {}

void WhileObject::putOnStack() {
	if (base) free(); // free any existing register

	base = program->getBPRegister();
	space = program->allocateStack(16);

	writeMem();
}

void WhileObject::pushStack() {
	if ( initialised ) {
		program->addInstruction( "text", make_shared<InstrPush>( valueRef() ) );
		program->addInstruction( "text", make_shared<InstrPush>( tagRef() ) );
	}
	else {
		program->addInstruction( "text", make_shared<InstrPush>( valueDirect() ) );
		program->addInstruction( "text", make_shared<InstrPush>( tagDirect() ) );
	}
}

shared_ptr<X86Register> WhileObject::attachRegister() {
	shared_ptr<X86Register> r = program->getFreeRegister();

	// update ref, free any existing ref
	if (ref) {
		r->assign( ref );

		if (initialised) {
			ref->free();
		}
	}
	ref = r->ref();

	return r;
}

void WhileObject::setLocation( shared_ptr<X86Register> r ) {
	base = r;
	initialised = true;
}

void WhileObject::setLocation( shared_ptr<X86Register> r, StackSpace s ) {
	base = r;
	space = s;
	initialised = true;
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
	initialised = true;
}

shared_ptr<X86Reference> WhileObject::tagDirect() {
	if (type) return type;
	else return tagRef();
}

shared_ptr<X86Reference> WhileObject::valueDirect() {
	if (ref) return ref;
	else return valueRef();
}

// function to put address in register?
// or return address as reference then register.assign(addr)
shared_ptr<X86Reference> WhileObject::addrRef() {
	shared_ptr<X86Register> r = program->getFreeRegister();

	if (!initialised) {
		r->assign( program->getSPRegister()->ref() );
		r->add( make_shared<X86Reference>(-16) );
		pushStack();

	}
	else {
		// TODO can use: leaq	-16(%rbp), %rax
		r->assign( base->ref() );
		if (space.begin != 0) r->add( make_shared<X86Reference>(space.begin) );
	}

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
	r1->add( make_shared<X86Reference>(1) );
	r1->multiply( make_shared<X86Reference>(16) );

	// TODO not writing may lose address
	shared_ptr<X86Register> r2 = program->malloc( r1->ref() );
	r1->free();
	ref = r2->ref();
	type = make_shared<X86Reference>(8);


	// set length
	shared_ptr<WhileObject> l = make_shared<WhileObject>(program);
	length(l);
	program->addInstruction( "text", make_shared<InstrMov>( make_shared<X86Reference>(4), l->tagRef() ) );
	program->addInstruction( "text", make_shared<InstrMov>( v, l->valueRef() ) );
	l->free();

	if (write) {
		writeMem();
	}

	//r2->free(); -- free this object instead
}

void WhileList::length(shared_ptr<WhileObject> inout) {
	shared_ptr<X86Register> r = program->getFreeRegister();
	r->assign( valueDirect() );
	inout->setLocation(r);
	//return make_shared<WhileObject>(program, r, 4);
}

void WhileList::get(shared_ptr<WhileObject> inout, shared_ptr<X86Reference> ref) {
	shared_ptr<X86Register> r = program->getFreeRegister();
	r->assign( ref );
	r->add( make_shared<X86Reference>(1) );
	r->multiply( make_shared<X86Reference>(16) );
	r->setSize(8);
	r->add( valueDirect() );
	inout->setLocation(r);
	//return make_shared<WhileObject>(program, r, inner_type);
}

WhileRecord::WhileRecord( shared_ptr<X86Program> p ): WhileObject(p) {}
WhileRecord::~WhileRecord() {}

void WhileRecord::initialise(shared_ptr<X86Reference> v, bool write) {
	// this object must free r

	// structure:
	// number of elements - int
	// pairs of string -> any type
	// size = 16 + n * 32

	shared_ptr<X86Register> r = program->malloc( make_shared<X86Reference>(64) );
	ref = r->ref();
	type = make_shared<X86Reference>(16);


	if (write) {
		writeMem();
	}
}

} /* namespace std */
