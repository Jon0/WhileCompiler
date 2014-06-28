/*
 * JavaReference.h
 *
 *  Created on: 19/06/2014
 *      Author: asdf
 */

#ifndef JAVAREFERENCE_H_
#define JAVAREFERENCE_H_

#include "Bytecode.h"

namespace std {

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

} /* namespace std */

#endif /* JAVAREFERENCE_H_ */
