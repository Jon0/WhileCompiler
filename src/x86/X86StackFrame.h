/*
 * X86StackFrame.h
 *
 *  Created on: 29/05/2014
 *      Author: remnanjona
 */

#ifndef X86STACKFRAME_H_
#define X86STACKFRAME_H_

#include <map>
#include <memory>

#include "X86Reference.h"
#include "X86Instruction.h"
#include "X86Program.h"

namespace std {

class X86StackFrame {
public:
	X86StackFrame();
	virtual ~X86StackFrame();

	/*
	 * return instructions to apply actions in assembly
	 */
	shared_ptr<X86Instruction> allocate() {
		initial = make_shared<InstrSub>("$0", "%rsp");
		return initial;
	}

	mem_space nextSpace(shared_ptr<X86Program> p, unsigned int s) {
		size += s;

		// modify initial
		if (initial) {
			initial->modifyLHS("$"+to_string(size));
		}



		shared_ptr<X86RegAddrRef> refr = make_shared<X86RegAddrRef>(p->getBPRegister(), -size, p->addrSize());
		mem_space ss { refr, s };
		return ss;
	}

	void clear() {
		size = 0;
		initial = NULL;
	}

private:
	unsigned int size;

	shared_ptr<InstrSub> initial;

	map<string, mem_space> spaces; // TODO label spaces

};

} /* namespace std */

#endif /* X86STACKFRAME_H_ */
