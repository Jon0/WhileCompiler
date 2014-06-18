/*
 * JavaInstruction.h
 *
 *  Created on: 19/06/2014
 *      Author: asdf
 */

#ifndef JAVAINSTRUCTION_H_
#define JAVAINSTRUCTION_H_

#include <vector>

#include "JavaReference.h"

namespace std {

enum IName {
	ifeq,
	ldc
};

typedef vector<JavaReference> IArgs;

class JavaInstruction {
public:
	JavaInstruction(IName, IArgs);
	virtual ~JavaInstruction();

	vector<char> toBytecode();

private:
	IName name;
	IArgs args;
};

} /* namespace std */

#endif /* JAVAINSTRUCTION_H_ */
