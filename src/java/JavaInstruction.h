/*
 * JavaInstruction.h
 *
 *  Created on: 19/06/2014
 *      Author: asdf
 */

#ifndef JAVAINSTRUCTION_H_
#define JAVAINSTRUCTION_H_

#include <memory>
#include <string>
#include <vector>

#include "Bytecode.h"
#include "JavaReference.h"

namespace std {

enum IName {
	getstatic = 0xb2,
	iadd = 0x60,
	ifeq = 0x99,
	invoke = 0xb6,
	ldc = 0x12,
	ret = 0xb1
};

typedef vector<shared_ptr<JavaReference>> IArgs;

class JavaInstruction {
public:
	JavaInstruction(IName);
	JavaInstruction(IName, IArgs);
	JavaInstruction(IName, shared_ptr<JavaReference>);
	virtual ~JavaInstruction();

	short size();
	bytecode toBytecode();

private:
	IName name;
	IArgs args;
};

} /* namespace std */

#endif /* JAVAINSTRUCTION_H_ */
