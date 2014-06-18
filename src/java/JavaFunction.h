/*
 * JavaFunction.h
 *
 *  Created on: 19/06/2014
 *      Author: remnanjona
 */

#ifndef JAVAFUNCTION_H_
#define JAVAFUNCTION_H_

#include <memory>

namespace std {

class Classfile;

class JavaFunction {
public:
	JavaFunction();
	virtual ~JavaFunction();

	void writeByteCode( shared_ptr<Classfile> );
};

} /* namespace std */

#endif /* JAVAFUNCTION_H_ */
