/*
 * X86Program.h
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#ifndef X86PROGRAM_H_
#define X86PROGRAM_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "X86StackFrame.h"

namespace std {

class X86Function;
class X86Instruction;
class X86Reference;
class X86Register;

typedef map<string, shared_ptr<X86Function>> function_map;
typedef vector< shared_ptr<X86Reference> > arg_list;

class X86Program: public enable_shared_from_this<X86Program> {
public:
	X86Program();
	virtual ~X86Program();

	int addrSize();
	string getName();
	vector<string> getSections();
	vector<shared_ptr<X86Instruction>> getInstructions( string );

	void initialise( string );

	StackSpace allocateStack(int);

	void beginFunction( string );
	void endFunction();

	void callFunction( shared_ptr<X86Function>, arg_list );
	shared_ptr<X86Register> malloc( shared_ptr<X86Reference> );

	shared_ptr<X86Register> getFreeRegister();
	shared_ptr<X86Register> getBPRegister();

	string availableRegisters(); // debug register use

	// make this private
	void addInstruction( string, shared_ptr<X86Instruction> );



private:
	string name;

	shared_ptr<X86Function> currentF;
	function_map functions;
	shared_ptr<X86Function> malloc_func;

	map<string, vector<shared_ptr<X86Instruction>>> sections;
	map<string, shared_ptr<X86Instruction>> labels;

	// program state
	vector<shared_ptr<X86Register>> pool;
	shared_ptr<X86Register> ax;
	shared_ptr<X86Register> bx;
	shared_ptr<X86Register> cx;
	shared_ptr<X86Register> dx;
	shared_ptr<X86Register> di;
	shared_ptr<X86Register> sp;
	shared_ptr<X86Register> bp;


	shared_ptr<X86StackFrame> stack;

	int addr_size;

	shared_ptr<X86Register> makeRegister(string, bool);
};

} /* namespace std */

#endif /* X86PROGRAM_H_ */
