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
	shared_ptr<X86Register> newbase = program->getBPRegister();
	StackSpace newspace = program->allocateStack(16);

	if (base) {
		shared_ptr<X86Register> r = program->getFreeRegister();
		r->setSize(8);
		program->addInstruction( "text", make_shared<InstrMov>( tagRef(), r->ref() ) );
		program->addInstruction( "text", make_shared<InstrMov>( r->ref(), make_shared<X86Reference>( newbase, newspace.begin, 8 ) ) );

		program->addInstruction( "text", make_shared<InstrMov>( valueRef(), r->ref() ) );
		program->addInstruction( "text", make_shared<InstrMov>( r->ref(), make_shared<X86Reference>( newbase, newspace.begin + 8, 8 ) ) );
		r->free();
		base->free(); // free any existing base register
	}

	base = newbase;
	space = newspace;

	// write stored items
	if (ref) {
		writeMem();
	}
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
	shared_ptr<X86Register> r;
	if (ref) {
		if ( ref->isRegister() ) {
			return ref->getRegister();
		}

		r = program->getFreeRegister();
		r->assign( ref );

		// before update ref, free any existing ref
		if (initialised) {
			ref->free();
		}
	}
	else {
		r = program->getFreeRegister();

		// copy base value if available
		if (base) {
			r->assign( valueRef() );
		}
	}

	ref = r->ref();
	return r;

}

shared_ptr<X86Register> WhileObject::attachRegisterType() {
	shared_ptr<X86Register> r;
	if (type) {
		if ( type->isRegister() ) {
			return type->getRegister();
		}

		r = program->getFreeRegister();
		r->assign( type );

		// before update ref, free any existing ref
		if (initialised) {
			type->free();
		}
	}
	else {
		r = program->getFreeRegister();

		// copy base value if available
		if (base) {
			r->assign( tagRef() );
		}
	}

	type = r->ref();
	return r;

}

/**
 * set an existing location
 */
void WhileObject::setLocation( shared_ptr<X86Register> r ) {
	base = r;
	ref = NULL;
	type = NULL;
	initialised = true;
}

void WhileObject::setLocation( shared_ptr<X86Register> r, StackSpace s ) {
	base = r;
	space = s;
	ref = NULL;
	type = NULL;
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

/**
 * does not write to memory
 */
void WhileObject::modifyType(int t) {
	type = make_shared<X86Reference>(t);
}

void WhileObject::free() {
	if (ref) ref->free();
	ref = NULL;
	if (type) type->free();
	type = NULL;
	if (base) base->free();
}

void WhileObject::writeMem() {
	if (!ref) {
		cout << "error writing memory" << endl;
		return;
	}

	shared_ptr<X86Register> r = program->getFreeRegister();
	r->setSize(8);
	program->addInstruction( "text", make_shared<InstrMov>( type, r->ref() ) );
	program->addInstruction( "text", make_shared<InstrMov>( r->ref(), tagRef() ) );

	program->addInstruction( "text", make_shared<InstrMov>( ref, r->ref() ) );
	program->addInstruction( "text", make_shared<InstrMov>( r->ref(), valueRef() ) );
	r->free();

	// should not be used once assigned a memory location
	ref->free();
	type->free();
	ref = NULL;
	type = NULL;

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
		base->setSize(8);
		r->assign( base->ref() );
		if (space.begin != 0) r->add( make_shared<X86Reference>(space.begin) );
	}

	return make_shared<X86Reference>( r, 8 );
}

shared_ptr<X86Reference> WhileObject::tagRef() {
	base->setSize(8);
	return make_shared<X86Reference>( base, space.begin, 8 );
}

shared_ptr<X86Reference> WhileObject::valueRef() {
	base->setSize(8);
	return make_shared<X86Reference>( base, space.begin + 8, 8 );
}

string WhileObject::debug() {
		string result = "";
		if (ref) result += "[ref] "+ref->place()+"\t";
		if (type) result += "[type] "+type->place()+"\t";
		if (base) result += "[base] "+base->place()+"\t";
		return result;
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
