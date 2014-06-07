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

namespace std {

class WhileObject;
class X86Function;
class X86Instruction;
class X86Reference;
class X86RegAddrRef;
class X86Register;
class X86StackFrame;

typedef map<string, shared_ptr<X86Function>> function_map;
typedef vector<shared_ptr<X86Function>> function_list;
typedef vector< shared_ptr<X86Reference> > arg_list;
typedef vector< shared_ptr<WhileObject> > obj_list;

struct mem_space {
	shared_ptr<X86RegAddrRef> ref;
	unsigned int size;
};

class X86Program: public enable_shared_from_this<X86Program> {
public:
	X86Program();
	virtual ~X86Program();

	int addrSize();
	string getName();
	vector<string> getSections();
	vector<shared_ptr<X86Instruction>> getInstructions( string );

	void initialise( string );

	mem_space allocateStack(int);

	void declareFunctions(function_list);
	void beginFunction( string, bool );
	void endFunction();

	shared_ptr<X86Function> getFunction( string );
	shared_ptr<X86Register> callFunction( shared_ptr<X86Function>, arg_list );
	shared_ptr<WhileObject> callFunction( shared_ptr<X86Function>, obj_list );
	shared_ptr<X86Register> malloc( shared_ptr<X86Reference> );

	shared_ptr<X86Register> getFreeRegister();
	shared_ptr<X86Register> getDIRegister();
	shared_ptr<X86Register> getBPRegister();
	shared_ptr<X86Register> getSPRegister();

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
	shared_ptr<X86Register> si;
	shared_ptr<X86Register> di;
	shared_ptr<X86Register> sp;
	shared_ptr<X86Register> bp;


	shared_ptr<X86StackFrame> stack;

	int addr_size;

	shared_ptr<X86Register> makeRegister(string, bool);
};

} /* namespace std */

#endif /* X86PROGRAM_H_ */
