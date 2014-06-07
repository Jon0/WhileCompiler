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
	space.ref = 0;
	space.size = 0;
	type = 0;
	initialised = false;
}

WhileObject::~WhileObject() {}

void WhileObject::putOnStack() {
	mem_space newspace = program->allocateStack(16);

	// if the object already has a memory location
	if (initialised) {
		shared_ptr<X86Register> r = program->getFreeRegister();
		r->setSize(8);
		program->addInstruction( "text", make_shared<InstrMov>( tagRef(), r->ref() ) );
		program->addInstruction( "text", make_shared<InstrMov>( r->ref(), newspace.ref ) );
		program->addInstruction( "text", make_shared<InstrMov>( valueRef(), r->ref() ) );
		program->addInstruction( "text", make_shared<InstrMov>( r->ref(), newspace.ref->index(8) ) );
		space.ref->free(); // free any existing register
	}

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
			shared_ptr<X86RegRef> regref = static_pointer_cast<X86RegRef, X86Reference>( ref );
			return regref->getRegister();
		}

		r = program->getFreeRegister();
		r->assign( ref );
	}
	else {
		r = program->getFreeRegister();

		// copy base value if available
		if (initialised) {
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
			shared_ptr<X86RegRef> regref = static_pointer_cast<X86RegRef, X86Reference>( ref );
			return regref->getRegister();
		}

		r = program->getFreeRegister();
		r->assign( type );
	}
	else {
		r = program->getFreeRegister();

		// copy base value if available
		if (initialised) {
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
	space = mem_space{ r->ref(0), 0 };
	ref = NULL;
	type = NULL;
	initialised = true;
}

void WhileObject::setLocation( mem_space m ) {
	space = m;
	ref = NULL;
	type = NULL;
	initialised = true;
}

void WhileObject::initialise( shared_ptr<X86Reference> v, int t, bool write ) {
	ref = v;
	type = make_shared<X86ConstRef>(t);
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
	type = make_shared<X86ConstRef>(t);
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


	// should not be used once assigned a memory location
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

// function to put address in register
shared_ptr<X86RegRef> WhileObject::addrRef() {
	shared_ptr<X86Register> r = program->getFreeRegister();
	if (!initialised) {
		r->assign( program->getSPRegister()->ref() );
		r->add( make_shared<X86ConstRef>(-16) );
		pushStack();

	}
	else {
		r->assignAddrOf(space.ref);
	}

	return make_shared<X86RegRef>( r, 8 );
}

shared_ptr<X86Reference> WhileObject::tagRef() {
	if (space.ref) return space.ref;
	else return make_shared<X86ConstRef>(0);
}

shared_ptr<X86Reference> WhileObject::valueRef() {
	if (space.ref) return space.ref->index(8);
	else return make_shared<X86ConstRef>(0);
}

string WhileObject::debug() {
		string result = "";
		if (ref) result += "[ref] "+ref->place()+"\t";
		if (type) result += "[type] "+type->place()+"\t";
		if (space.ref) result += "[base] "+space.ref->place(0)+"\t";
		return result;
}

WhileList::WhileList( shared_ptr<X86Program> p ): WhileObject(p) {}
WhileList::~WhileList() {}

void WhileList::initialise(shared_ptr<X86Reference> v, bool write) {
	shared_ptr<X86Register> r1 = program->getFreeRegister();
	r1->assign( v );
	r1->add( make_shared<X86ConstRef>(1) );
	r1->multiply( make_shared<X86ConstRef>(16) );

	// TODO not writing may lose address
	shared_ptr<X86Register> r2 = program->malloc( r1->ref() );
	ref = r2->ref();
	type = make_shared<X86ConstRef>(8);


	// set length
	shared_ptr<WhileObject> l = make_shared<WhileObject>(program);
	length(l);
	program->addInstruction( "text", make_shared<InstrMov>( make_shared<X86ConstRef>(4), l->tagRef() ) );
	program->addInstruction( "text", make_shared<InstrMov>( v, l->valueRef() ) );

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
	r->add( make_shared<X86ConstRef>(1) );
	r->multiply( make_shared<X86ConstRef>(16) );
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

	shared_ptr<X86Register> r = program->malloc( make_shared<X86ConstRef>(64) );
	ref = r->ref();
	type = make_shared<X86ConstRef>(16);

	if (write) {
		writeMem();
	}
}

} /* namespace std */
