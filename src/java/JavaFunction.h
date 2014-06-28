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

class Classfile;

class JavaFunction {
public:
	JavaFunction();
	virtual ~JavaFunction();

	unsigned int codeSize();
	void addInstruction(JavaInstruction);
	bytecode writeByteCode( shared_ptr<Classfile> );

private:
	vector<JavaInstruction> inst_list;
};

} /* namespace std */

#endif /* JAVAFUNCTION_H_ */
