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
#include "X86Register.h"

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
	X86Reference(int);
	X86Reference(shared_ptr<X86Register>);
	virtual ~X86Reference();

	string place();
	string stackPlace();

	/*
	 * return instructions to apply actions in assembly
	 */
	shared_ptr<X86Instruction> setValue(string);
	shared_ptr<X86Instruction> assignRegister(shared_ptr<X86Register>);

private:
	// list places stored
	// TODO currently only registers...
	shared_ptr<X86Register> reg;

	int stackPlaceOffset;
	bool isOnStack;

	string constant;

};

} /* namespace std */

#endif /* X86REFERENCE_H_ */
