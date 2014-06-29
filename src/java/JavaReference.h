/*
 * JavaReference.h
 *
 *  Created on: 19/06/2014
 *      Author: asdf
 */

#ifndef JAVAREFERENCE_H_
#define JAVAREFERENCE_H_

#include <memory>

#include "Bytecode.h"

namespace std {

class JavaInstruction;

/*
 * reference constant pool or other bytecode args
 */
class JavaReference {
public:
	JavaReference();
	virtual ~JavaReference();

	virtual unsigned char size() = 0;

	virtual bytecode toByteCode() = 0;
};

class ConstReference: public JavaReference {
public:
	ConstReference(short);
	ConstReference(short, unsigned char);
	virtual ~ConstReference();

	virtual unsigned char size();

	virtual bytecode toByteCode();

private:
	unsigned char ref_size;
	short addr;
};

class JumpReference: public JavaReference {
public:
	JumpReference(shared_ptr<JavaInstruction>);
	virtual ~JumpReference();

	virtual unsigned char size();

	virtual bytecode toByteCode();

private:
	shared_ptr<JavaInstruction> target;
};

} /* namespace std */

#endif /* JAVAREFERENCE_H_ */
