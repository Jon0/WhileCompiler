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

#include "X86Instruction.h"

namespace std {

class Type;

struct StackSpace {
	int begin, size;
	shared_ptr<Type> type;
};

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

	StackSpace nextSpace(shared_ptr<Type> t, int s) {
		size += s;

		// modify initial
		initial->modifyLHS("$"+to_string(size));


		StackSpace ss { -size, s, t };
		return ss;
	}

	void clear() {
		size = 0;
		initial = NULL;
	}

private:
	int size;

	shared_ptr<InstrSub> initial;

	map<string, StackSpace> spaces;

};

} /* namespace std */

#endif /* X86STACKFRAME_H_ */
