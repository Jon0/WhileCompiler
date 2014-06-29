/*
 * JavaFunction.h
 *
 *  Created on: 19/06/2014
 *      Author: remnanjona
 */

#ifndef JAVAFUNCTION_H_
#define JAVAFUNCTION_H_

#include <memory>

#include "JavaInstruction.h"

namespace std {

class ConstantPool;

class JavaFunction {
public:
	JavaFunction( shared_ptr<ConstantPool> );
	virtual ~JavaFunction();

	unsigned int codeSize();
	unsigned int numLocals();

	void add(JavaInstruction);
	bytecode writeByteCode();

private:
	vector<JavaInstruction> inst_list;

	unsigned short name, descriptor, code_string;

	unsigned int num_locals;
};

} /* namespace std */

#endif /* JAVAFUNCTION_H_ */
