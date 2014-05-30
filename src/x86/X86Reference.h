/*
 * X86Reference.h
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#ifndef X86REFERENCE_H_
#define X86REFERENCE_H_

#include <memory>
#include <string>

#include "X86Instruction.h"
#include "X86Program.h"
#include "X86Register.h"
#include "X86StackFrame.h"

namespace std {

/*
 * gives access to variables
 * vars can be stored both in registers or on stack/heap memory
 * also includes labeled constants, and basic constants
 * register access is faster
 */
class X86Reference: public enable_shared_from_this<X86Reference> {
public:

	/*
	 * create reference to a constant integer value
	 * becomes $x in x86 assembly
	 */
	X86Reference(int);

	/*
	 *	%rax type locations
	 *	parameters register, bytesize
	 */
	X86Reference(shared_ptr<X86Register>, int);

	/*
	 *	-8(%rbp) type locations
	 *	parameters register, offset, bytesize
	 */
	X86Reference(shared_ptr<X86Register>, int, int);

	/*
	 * stack space reference
	 */
	X86Reference(shared_ptr<X86Register>, StackSpace);

	virtual ~X86Reference();

	bool isLive();
	void free();

	/*
	 * creates another reference based on offset from this
	 */
	shared_ptr<X86Reference> index(int, int);

	/*
	 * offset from register
	 */
	int getOffset();

	/*
	 * size of the reference
	 */
	int typeSize();

	/*
	 * return with priority on registers
	 */
	string place();
	string place(int);	// force type width

	/*
	 * return instructions to apply actions in assembly -- no longer used
	 */
	shared_ptr<X86Instruction> setValue(shared_ptr<X86Reference>);


	//void assignRegister(shared_ptr<X86Program>, shared_ptr<X86Register>);
	//void assignRegisterPointer(shared_ptr<X86Program>, shared_ptr<X86Register>);

	string debug() {
		return "X86Reference s:" + to_string(offset) + ", c:"
				+ constant + ", r:" + to_string(reg == NULL) + " (" + place() + ")";
	}

private:
	shared_ptr<X86Register> reg;
	int offset;
	int type_size;
	bool use_addr, is_live;

	string constant;
};

} /* namespace std */

#endif /* X86REFERENCE_H_ */
