/*
 * X86Program.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "../lang/Type.h"
#include "WhileObject.h"
#include "X86Function.h"
#include "X86Instruction.h"
#include "X86Program.h"
#include "X86Reference.h"
#include "X86Register.h"
#include "X86StackFrame.h"

namespace std {

X86Program::X86Program() {
	addr_size = 8;
	malloc_func = make_shared<X86Function>("malloc", true, true);
}

X86Program::~X86Program() {}

int X86Program::addrSize() {
	return addr_size;
}

string X86Program::getName() {
	return name;
}

vector<string> X86Program::getSections() {
	vector<string> a;
	for (auto v: sections) {
		a.push_back(v.first);
	}
	return a;
}

vector<shared_ptr<X86Instruction>> X86Program::getInstructions(string s) {
	return sections[s];
}

void X86Program::initialise(string n) {
	ax = makeRegister("ax", true);
	bx = makeRegister("bx", true);
	cx = makeRegister("cx", true);
	dx = makeRegister("dx", true);
	si = makeRegister("si", false);
	di = makeRegister("di", false);
	sp = makeRegister("sp", false);
	bp = makeRegister("bp", false);
	makeRegister("xmm0", true);
	makeRegister("xmm1", true);
	makeRegister("xmm2", true);
	makeRegister("xmm3", true);
	makeRegister("xmm4", true);
	makeRegister("xmm5", true);
	makeRegister("xmm6", true);
	makeRegister("xmm7", true);
	stack = make_shared<X86StackFrame>();

	name = n;
	addInstruction( "file", make_shared<InstrDirective>( "file", vector<string>({n}) ) );
	addInstruction( "text", make_shared<InstrDirective>( "text" ) );
}

mem_space  X86Program::allocateStack(int size) {
	return stack->nextSpace(shared_from_this(), size);
}

void X86Program::declareFunctions(function_list l) {
	for (shared_ptr<X86Function> f: l) {
		string fname = f->getName();
		functions.insert( function_map::value_type(fname, f ) );
	}
}

void X86Program::beginFunction( string name, bool has_ret ) {
	stack->clear();

	// label function
	addInstruction( "text", make_shared<InstrGlobl>( name ) );	//.globl [function name]
	addInstruction( "text", make_shared<InstrFuncLabel>( name ) );

	// save base pointer
	addInstruction( "text", make_shared<InstrPush>(make_shared<X86RegRef>(bp, addr_size)) );
	bp->assign( sp->ref() );
	addInstruction( "text", stack->allocate() ); // TODO should modify later
}

void X86Program::endFunction() {
	// same as leave instruction
	sp->assign( bp->ref() );
	addInstruction( "text", make_shared<InstrPop>( bp->ref() ) );
	addInstruction( "text", make_shared<InstrRet>() );
}

shared_ptr<X86Function> X86Program::getFunction( string s ) {
	return functions[s];
}

shared_ptr<X86Register> X86Program::callFunction( shared_ptr<X86Function> f, arg_list args ) {

	// save all in use registers
	reg_list stored = pushUsedRegisters();
	bool pad = false;


	// fix a really weird bug
	if ((stored.size() % 2) == 1) {
		pad = true;
		addInstruction( "text", make_shared<InstrPush>( make_shared<X86ConstRef>(0) ) );
	}

	// pass args in registers
	// TODO multiple args
	int count = 0;
	for ( shared_ptr<X86Reference> ref: args ) {
		if (count == args.size()-1) di->assign( ref ); // di will be the last arg
		else si->assign( ref );
		count++;

	}
	addInstruction( "text", make_shared<InstrCall>( f->getName() ) );

	// address returned in eax
	shared_ptr<X86Register> nr;
	if ( f->hasReturn() ) {
		int axSize = ax->getSize();
		ax->setSize(8);
		nr = getFreeRegister();
		nr->assign( ax->ref() );
		ax->setSize(axSize);
	}

	// restore used registers
	if (pad) addInstruction( "text", make_shared<InstrPop>( stored.back()->ref() ) );
	popRegisters(stored);

	return nr;
}

shared_ptr<WhileObject> X86Program::callFunction( shared_ptr<X86Function> f, shared_ptr<Type> rt, obj_list args ) {

	// push args and return space to stack -- before saving
	if (f->hasReturn()) {
		addInstruction( "text", make_shared<InstrPush>( make_shared<X86ConstRef>(0) ) );
		addInstruction( "text", make_shared<InstrPush>( make_shared<X86ConstRef>(0) ) );
	}
	for (shared_ptr<WhileObject> wo: args) {
		wo->pushStack();
	}

	// save all in use registers
	reg_list stored = pushUsedRegisters();

	// set di to pointer to block of args/return -- after saving registers since di is modified
	shared_ptr<X86Register> location = di;
	location->assign( sp->ref() );
	int relativePlace = 8*(stored.size()) + 16*(args.size() - 1);
	if (f->hasReturn())	relativePlace += 16;
	location->add( make_shared<X86ConstRef>(relativePlace) );


	// call instruction
	addInstruction( "text", make_shared<InstrCall>( f->getName() ) );


	// find the result -- it uses the modified rdi register -- must be before restore
	shared_ptr<WhileObject> returnPlace;
	if (f->hasReturn()) {
		returnPlace = make_obj( shared_from_this(), rt );
		shared_ptr<X86Register> reg = getFreeRegister();
		reg->assign( location->ref() );
		returnPlace->setLocation( reg, false );	// the returned object must free the register
	}

	// restore used registers
	popRegisters(stored);

	return returnPlace;
}

shared_ptr<X86Register> X86Program::malloc( shared_ptr<X86Reference> r ) {
	return callFunction( malloc_func, arg_list{ r } );
}

shared_ptr<X86Register> X86Program::getFreeRegister() {
	return getRegisterFromPool(pool);
}

shared_ptr<X86Register> X86Program::getFreeMmxRegister() {
	return getRegisterFromPool(mmx_pool);
}

shared_ptr<X86Register> X86Program::getDIRegister() {
	return di;
}

shared_ptr<X86Register> X86Program::getBPRegister() {
	return bp;
}

shared_ptr<X86Register> X86Program::getSPRegister() {
	return sp;
}

reg_list X86Program::pushUsedRegisters() {
	reg_list stored;
	for (shared_ptr<X86Register> r: pool) {
		if (r->inUse()) {
			stored.push_back(r);
			addInstruction( "text", make_shared<InstrPush>( r->ref() ) );
		}
	}
	stored.push_back(di);
	addInstruction( "text", make_shared<InstrPush>( di->ref() ) );
	return stored;
}

void X86Program::popRegisters(reg_list stored) {
	while (!stored.empty()) {
		addInstruction( "text", make_shared<InstrPop>( stored.back()->ref() ) );
		stored.pop_back();
	}
}

string X86Program::availableRegisters() {
	string a = "{";
	int i = 0;
	for (shared_ptr<X86Register> r: pool) {
		if (!r->inUse()) {
			a += r->getName();
			if (i < pool.size() - 1) a += ", ";
		}
		i++;
	}
	a += "}";
	return a;
}

void X86Program::addInstruction( string s, shared_ptr<X86Instruction> i ) {

	// add section if it doesnt exist
	if ( sections.count(s) == 0 ) {
		vector<shared_ptr<X86Instruction>> iv;
		sections.insert( map<string, vector<shared_ptr<X86Instruction>>>::value_type(s, iv) );
	}

	sections[s].push_back(i);
}

shared_ptr<X86Reference> X86Program::intdivide( shared_ptr<X86Reference> a, shared_ptr<X86Reference> b, bool rem ) {
	// push ax and dx

	shared_ptr<X86Register> result = getFreeRegister();
	result->assign(b);

	addInstruction( "text", make_shared<InstrPush>( ax->ref() ) );
	addInstruction( "text", make_shared<InstrPush>( dx->ref() ) );

	// cdq
	ax->assign(a);
	addInstruction( "text", make_shared<InstrCdq>() );

	// divide by b
	addInstruction( "text", make_shared<InstrDiv>( result->ref() ) );

	if (rem) {
		result->assign(dx->ref());
	}
	else {
		result->assign(ax->ref());
	}

	// restore ax and dx
	addInstruction( "text", make_shared<InstrPop>( dx->ref() ) );
	addInstruction( "text", make_shared<InstrPop>( ax->ref() ) );

	return result->ref();
}

shared_ptr<X86Register> X86Program::makeRegister(string s, bool add) {
	shared_ptr<X86Register> r = make_shared<X86Register>(shared_from_this(), s);
	if (add) {
		if (s[0] == 'x') mmx_pool.push_back(r);
		else pool.push_back(r);
	}
	return r;
}

shared_ptr<X86Register> X86Program::getRegisterFromPool(reg_list pl) {
	for (shared_ptr<X86Register> r: pl) {
		if (!r->inUse()) {
			r->setSize(8);
			return r;
		}
	}

	// save to stack
	cout << "out of registers" << endl;
	mem_space s = allocateStack(8);

	return ax;
}

} /* namespace std */
