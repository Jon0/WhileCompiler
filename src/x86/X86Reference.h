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
	X86Reference(string);
	X86Reference(shared_ptr<X86Register>, StackSpace);
	X86Reference(shared_ptr<X86Register>, int);
	virtual ~X86Reference();

	bool isPointer();

	int typeSize();

	int stackOffset();

	bool isLive(); // TODO is this reference usable - registers get updated it gets lost

	/*
	 * return with priority on registers
	 */
	string place();
	string place(int);	// force type width

	/*
	 * reference value prioritise non-register
	 */
	string stackPlace();

	shared_ptr<X86Reference> index(int, int);

	/*
	 * return instructions to apply actions in assembly
	 */
	shared_ptr<X86Instruction> setValue(shared_ptr<X86Reference>);
	shared_ptr<X86Instruction> assignRegister(shared_ptr<X86Register>);

	void assignRegisterPointer(shared_ptr<X86Program>, shared_ptr<X86Register>);

	string debug() {
		return "X86Reference s:" + to_string(placement.begin) + ", c:"
				+ constant + ", r:" + to_string(reg == NULL) + " (" + place() + ")";
	}

private:
	// list places stored
	// TODO currently only registers...
	shared_ptr<X86Register> reg;

	shared_ptr<X86Register> stackBase;
	StackSpace placement;
	bool isOnStack;

	string constant;

	int type_size;

};

} /* namespace std */

#endif /* X86REFERENCE_H_ */
