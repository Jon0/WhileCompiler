/*
 * X86Program.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "X86Function.h"
#include "X86Instruction.h"
#include "X86Program.h"
#include "X86Reference.h"
#include "X86Register.h"

namespace std {

X86Program::X86Program() {
	addr_size = 8;
	malloc_func = make_shared<X86Function>("malloc", true);
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
	di = makeRegister("di", false);
	sp = makeRegister("sp", false);
	bp = makeRegister("bp", false);
	stack = make_shared<X86StackFrame>();

	name = n;
	addInstruction( "file", make_shared<InstrDirective>( "file", vector<string>({n}) ) );
	addInstruction( "text", make_shared<InstrDirective>( "text" ) );
}

StackSpace X86Program::allocateStack(int size) {
	return stack->nextSpace(size);
}

void X86Program::beginFunction( string name ) {
	stack->clear();
	functions.insert( function_map::value_type(name, make_shared<X86Function>(name, false) ) );

	// label function
	addInstruction( "text", make_shared<InstrGlobl>( name ) );	//.globl [function name]
	addInstruction( "text", make_shared<InstrFuncLabel>( name ) );

	// save base pointer
	addInstruction( "text", make_shared<InstrPush>(make_shared<X86Reference>(bp, addr_size)) );
	bp->assign( sp->ref() );
	addInstruction( "text", stack->allocate() ); // TODO should modify later
}

void X86Program::endFunction() {
	sp->assign( bp->ref() );
	addInstruction( "text", make_shared<InstrPop>("%rbp") );
	addInstruction( "text", make_shared<InstrRet>() );
}

shared_ptr<X86Function> X86Program::getFunction( string s ) {
	return functions[s];
}

void X86Program::callFunction( shared_ptr<X86Function> f, arg_list args ) {
	for ( shared_ptr<X86Reference> ref: args ) {
		di->assign( ref );
	}

	addInstruction( "text", make_shared<InstrCall>( f->getName() ) );
}

shared_ptr<X86Register> X86Program::malloc( shared_ptr<X86Reference> r ) {
	if (ax->inUse()) {
		addInstruction( "text", make_shared<InstrPush>( ax->ref() ) );
	}

	arg_list args;
	args.push_back( r );
	callFunction( malloc_func, args);

	// address returned in eax
	if (ax->inUse()) {
		int axSize = ax->getSize();
		ax->setSize(8);
		shared_ptr<X86Register> nr = getFreeRegister();
		nr->assign( ax->ref() );
		addInstruction( "text", make_shared<InstrPop>( ax->ref()->place() ) );
		ax->setSize(axSize);
		return nr;
	}
	else {
		ax->setSize(8);
		ax->setAsUsed();
		return ax;
	}
}

shared_ptr<X86Register> X86Program::getFreeRegister() {
	for (shared_ptr<X86Register> r: pool) {
		if (!r->inUse()) {
			return r;
		}
	}

	// TODO save to stack
	cout << "out of registers" << endl;
	return ax;
}

shared_ptr<X86Register> X86Program::getBPRegister() {
	return bp;
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

shared_ptr<X86Register> X86Program::makeRegister(string s, bool add) {
	shared_ptr<X86Register> r = make_shared<X86Register>(shared_from_this(), s);
	if (add) pool.push_back(r);
	return r;
}

} /* namespace std */
