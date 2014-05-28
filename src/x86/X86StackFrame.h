/*
 * X86StackFrame.h
 *
 *  Created on: 29/05/2014
 *      Author: remnanjona
 */

#ifndef X86STACKFRAME_H_
#define X86STACKFRAME_H_

#include <memory>

#include "X86Instruction.h"

namespace std {

class X86StackFrame {
public:
	X86StackFrame();
	virtual ~X86StackFrame();

	/*
	 * return instructions to apply actions in assembly
	 */
	shared_ptr<X86Instruction> allocate(int s) {
		initial = make_shared<InstrSub>("$"+to_string(s), "%rsp");
		return initial;
	}

	int nextSpace(int s) {
		size += s;

		// TODO modify initial

		return -size;
	}

private:
	int size;

	shared_ptr<InstrSub> initial;

};

} /* namespace std */

#endif /* X86STACKFRAME_H_ */
