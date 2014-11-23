/*
 * JavaObject.h
 *
 *  Created on: 1/07/2014
 *      Author: asdf
 */

#ifndef JAVAOBJECT_H_
#define JAVAOBJECT_H_

#include <memory>
#include <vector>

namespace std {

class ConstantPool;
class JavaFunction;

class JavaObject {
public:
	JavaObject();
	virtual ~JavaObject();

	virtual void print() = 0;
};

class JavaNumber: public JavaObject, public enable_shared_from_this<JavaNumber> {
public:
	JavaNumber(shared_ptr<JavaFunction>, short, short);
	JavaNumber(shared_ptr<JavaFunction>, shared_ptr<ConstantPool>, int);
	virtual ~JavaNumber();

	void push();

	shared_ptr<JavaNumber> add(shared_ptr<JavaNumber>);

	virtual void print();

private:
	shared_ptr<JavaFunction> func;
	short addr, pstream_addr, print_addr;

	shared_ptr<JavaNumber> lhs, rhs;
	shared_ptr<JavaInstruction> inst;
};

class JavaString {
public:
	JavaString(shared_ptr<JavaFunction>, shared_ptr<ConstantPool>);
	virtual ~JavaString();

	virtual void print();

private:
	shared_ptr<JavaFunction> func;
	short addr, pstream_addr, print_addr;
};

} /* namespace std */

#endif /* JAVAOBJECT_H_ */
