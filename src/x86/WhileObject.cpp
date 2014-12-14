/*
 * WhileObject.cpp
 *
 *  Created on: 30/05/2014
 *      Author: remnanjona
 */

#include "../lang/Type.h"
#include "WhileObject.h"

#include "X86Function.h"
#include "X86Program.h"
#include "X86Reference.h"
#include "X86Register.h"

namespace std {

WhileObject::WhileObject( shared_ptr<WhileObject> wo ) {
	program = wo->program;
	w_type = wo->w_type;
	space.ref = wo->space.ref;
	space.size = wo->space.size;
	read_only = wo->read_only;
	initialised = wo->initialised;
	ref = wo->ref;
	type = wo->type;
}

WhileObject::WhileObject( shared_ptr<X86Program> p, shared_ptr<lang::Type> wt ) {
	program = p;
	w_type = wt;
	space.ref = 0;
	space.size = 0;
	read_only = false;
	initialised = false;
	ref = NULL;
	type = NULL;
}

WhileObject::~WhileObject() {}

shared_ptr<lang::Type> WhileObject::getType() const {
	return w_type;
}

bool WhileObject::isReadOnly() {
	return read_only;
}

void WhileObject::setReadOnly() {
	read_only = true;
}

void WhileObject::putOnStack() {
	mem_space newspace = program->allocateStack(16);

	// if the object already has a memory location
	//if (initialised) {
		shared_ptr<X86Reference> r = program->getFreeRegister()->ref();
		program->addInstruction( "text", make_shared<InstrMov>( tagDirect(), r ) );
		program->addInstruction( "text", make_shared<InstrMov>( r, newspace.ref ) );
		program->addInstruction( "text", make_shared<InstrMov>( valueDirect(), r ) );
		program->addInstruction( "text", make_shared<InstrMov>( r, newspace.ref->index(8) ) );
		//space.ref->free(); // free any existing register
	//}


	space = newspace;
	ref = NULL;
	type = NULL;
	initialised = true;
}

void WhileObject::pushStack() {
	if ( initialised ) {
		program->addInstruction( "text", make_shared<InstrPush>( valueRef() ) );
		program->addInstruction( "text", make_shared<InstrPush>( tagRef() ) );
	}
	else {
		if (w_type->nameStr() == "real") {
			shared_ptr<X86Register> r = program->getFreeRegister();
			r->assign(valueDirect());
			program->addInstruction( "text", make_shared<InstrPush>( r->ref() ) );
			program->addInstruction( "text", make_shared<InstrPush>( tagDirect() ) );
		}
		else {
			program->addInstruction( "text", make_shared<InstrPush>( valueDirect() ) );
			program->addInstruction( "text", make_shared<InstrPush>( tagDirect() ) );
		}


	}
}

void WhileObject::attachRegister(shared_ptr<X86Register> r) {
	r->assign( valueDirect() );
	ref = r->ref();
}

void WhileObject::attachRegisterType(shared_ptr<X86Register> r) {
	r->assign( tagDirect() );
	ref = r->ref();
}

shared_ptr<X86Register> WhileObject::attachRegister() {
	if (ref) {
		if ( ref->isRegister() ) {
			shared_ptr<X86RegRef> regref = static_pointer_cast<X86RegRef, X86Reference>( ref );
			return regref->getRegister();
		}
	}

	// if type is real use mmx register
	shared_ptr<X86Register> r;
	if (w_type->nameStr() == "real") {
		r = program->getFreeMmxRegister();
	}
	else {
		r = program->getFreeRegister();
	}

	attachRegister( r );
	return r;
}

shared_ptr<X86Register> WhileObject::attachRegisterType() {
	if (type) {
		if ( type->isRegister() ) {
			shared_ptr<X86RegRef> regref = static_pointer_cast<X86RegRef, X86Reference>( type );
			return regref->getRegister();
		}
	}
	shared_ptr<X86Register> r = program->getFreeRegister();
	attachRegisterType( r );
	return r;
}


/**
 * set an existing location
 */
void WhileObject::setLocation( shared_ptr<X86Register> r, bool write ) {
	space = mem_space{ r->ref(0), 0 };

	if (write) {
		writeMem();
	}
	ref = NULL;
	type = NULL;
	initialised = true;
}

void WhileObject::setLocation( mem_space m, bool write ) {
	space = m;

	if (write) {
		writeMem();
	}
	ref = NULL;
	type = NULL;
	initialised = true;
}

void WhileObject::initialise( shared_ptr<X86Reference> v, bool write ) {
	ref = v;
	type = make_shared<X86ConstRef>(getTypeTag(w_type));

	if ( write ) {
		writeMem();
	}
}

void WhileObject::assign( shared_ptr<WhileObject> other, bool write ) {
	ref = other->valueDirect();
	type = other->tagDirect();

	if ( write ) {
		writeMem();
	}
}

void WhileObject::assign( shared_ptr<X86Reference> other, bool write ) {
	ref = other;

	if ( write ) {
		writeMem();
	}
}

void WhileObject::assignType( shared_ptr<lang::Type> t, bool write ) {
	if (t->nameStr() == "real") {
		shared_ptr<X86Register> r = program->getFreeMmxRegister();
		r->assign_itof( valueDirect() );
		ref = r->ref();
	}

	w_type = t;
	type = make_shared<X86ConstRef>(getTypeTag(t));

	if ( write ) {
		writeMem();
	}
}

void WhileObject::writeMem() {
	shared_ptr<X86Register> reg = program->getFreeRegister();
	reg->setSize(8);

	// TODO check *type != *tagRef() -- already assigned
	if (type) {
		reg->assign( type );
		program->addInstruction( "text", make_shared<InstrMov>( reg->ref(), tagRef() ) );
		type = NULL; 	// should not be used once assigned a memory location
	}

	if (ref) {
		reg->assign( ref );
		program->addInstruction( "text", make_shared<InstrMov>( reg->ref(), valueRef() ) );
		ref = NULL;
	}

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

shared_ptr<X86Reference> WhileObject::tagRef() {
	if (space.ref) return space.ref;
	else return make_shared<X86ConstRef>(getTypeTag(w_type));
}

shared_ptr<X86Reference> WhileObject::valueRef() {
	if (space.ref) return space.ref->index(8);
	else return make_shared<X86ConstRef>(0);
}

// function to put address in register
shared_ptr<X86RegRef> WhileObject::addrRef() {
	shared_ptr<X86Register> r = program->getFreeRegister();
	if (!initialised) {
		putOnStack();
	}

	r->assignAddrOf(space.ref);

	return make_shared<X86RegRef>( r, 8 );
}

void WhileObject::print() {
	program->callFunction(printFunc, arg_list{addrRef()});
}

shared_ptr<WhileObject> WhileObject::clone() {
	shared_ptr<WhileObject> obj = make_obj(program, w_type);

	if (w_type->isAtomic() || w_type->isNull()) {
		obj->assign( shared_from_this(), false );
	}
	else {
		shared_ptr<X86Register> r = program->callFunction(cloneFunc, arg_list{addrRef()});
		obj->setLocation( r, false );	// object result
		obj->putOnStack();
	}

	return obj;
}

shared_ptr<WhileObject> WhileObject::equiv( shared_ptr<WhileObject> other ) {
	// use a c function to compare objects
	shared_ptr<X86Register> r = program->callFunction(equivFunc, arg_list{addrRef(), other->addrRef()});
	shared_ptr<WhileObject> obj = make_shared<WhileObject>(program, lang::boolType); 	// bool result
	obj->initialise( r->ref(), false );
	return obj;

}

// only for lists
shared_ptr<WhileObject> WhileObject::append( shared_ptr<WhileObject> other ) {
	shared_ptr<X86Register> r = program->callFunction(appendFunc, arg_list{other->addrRef(), addrRef()});
	shared_ptr<WhileObject> obj = make_obj(program, w_type);
	obj->setLocation( r, false );	// object result

	return obj;
}

string WhileObject::debug() {
		string result = "";
		if (ref) result += "[ref] "+ref->place()+"\t";
		if (type) result += "[type] "+type->place()+"\t";
		if (space.ref) result += "[base] "+space.ref->place(0)+"\t";
		return result;
}

int WhileObject::getTypeSize(shared_ptr<lang::Type> t) {
	// all while types are 8+8 bytes
	return 16;
}

int WhileObject::getTypeTag(shared_ptr<lang::Type> t) {
	if ( t->nameStr() == "null") {
		return 1;
	}
	else if ( t->nameStr() == "bool") {
		return 2;
	}
	else if ( t->nameStr() == "char") {
		return 3;
	}
	else if ( t->nameStr() == "int") {
		return 4;
	}
	else if ( t->nameStr() == "real") {
		return 5;
	}
	else if (t->nameStr() == "string") {
		return 7;
	}
	else if (t->isList()) {
		return 8;
	}
	else if (t->isRecord()) {
		return 16;
	}
	return 0;
}

WhileList::WhileList( shared_ptr<WhileList> wo ):
			WhileObject( wo ) {
	inner_type = wo->inner_type;
}

WhileList::WhileList( shared_ptr<X86Program> p, shared_ptr<lang::Type> wt ):
		WhileObject(p, wt) {
	auto n = static_pointer_cast<lang::ListType, lang::Type>( wt );
	inner_type = n->innerType();
}

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
	shared_ptr<WhileObject> l = make_shared<WhileObject>(program, lang::intType);
	length(l);
	program->addInstruction( "text", make_shared<InstrMov>( make_shared<X86ConstRef>(4), l->tagRef() ) );
	program->addInstruction( "text", make_shared<InstrMov>( v, l->valueRef() ) );

	if (write) {
		writeMem();
	}
}

string WhileList::debug() {
	string result = "List: ";
	if (ref) result += "[ref] "+ref->place()+"\t";
	if (type) result += "[type] "+type->place()+"\t";
	if (space.ref) result += "[base] "+space.ref->place(0)+"\t";
	return result;
}

void WhileList::length(shared_ptr<WhileObject> inout) {
	shared_ptr<X86Register> r = program->getFreeRegister();
	r->assign( valueDirect() );
	inout->setLocation(r, false);
	//return make_shared<WhileObject>(program, r, 4);
}

void WhileList::get(shared_ptr<WhileObject> inout, shared_ptr<X86Reference> ref) {
	shared_ptr<X86Register> r = program->getFreeRegister();
	r->assign( ref );
	r->add( make_shared<X86ConstRef>(1) );
	r->multiply( make_shared<X86ConstRef>(16) );
	r->setSize(8);
	r->add( valueDirect() );
	inout->setLocation(r, false);
}


WhileRecord::WhileRecord( shared_ptr<WhileRecord> wo ):
			WhileObject( wo ) {}

WhileRecord::WhileRecord( shared_ptr<X86Program> p, shared_ptr<lang::Type> wt ):
		WhileObject(p, wt) {}

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

shared_ptr<WhileObject> make_obj(shared_ptr<X86Program> p, shared_ptr<lang::Type> wt) {
	if (wt->isList()) {
		return make_shared<WhileList>(p, wt);
	}
	else if (wt->isRecord()) {
		return make_shared<WhileRecord>(p, wt);
	}
	else {
		return make_shared<WhileObject>(p, wt);
	}
}

// TODO use clone instead
shared_ptr<WhileObject> copy_obj_readonly( shared_ptr<WhileObject> wo) {
	shared_ptr<WhileObject> copy;
	if (wo->getType()->isList()) {
		return make_shared<WhileList>( static_pointer_cast<WhileList, WhileObject>( wo ) );
	}
	else if (wo->getType()->isRecord()) {
		return make_shared<WhileRecord>( static_pointer_cast<WhileRecord, WhileObject>( wo ) );
	}
	else {
		return make_shared<WhileObject>(wo);
	}
	copy->setReadOnly();
	return copy;
}

} /* namespace std */
