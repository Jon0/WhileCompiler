/*
 * JavaObject.cpp
 *
 *  Created on: 1/07/2014
 *      Author: asdf
 */

#include "Constant.h"
#include "ConstantPool.h"
#include "JavaObject.h"

namespace std {

JavaObject::JavaObject() {
	// TODO Auto-generated constructor stub

}

JavaObject::~JavaObject() {
	// TODO Auto-generated destructor stub
}

JavaNumber::JavaNumber(shared_ptr<JavaFunction> f, short s, short p) {
	func = f;
	addr = -1;
	pstream_addr = s;
	print_addr = p;

}

JavaNumber::JavaNumber(shared_ptr<JavaFunction> f, shared_ptr<ConstantPool> cp, int i) {
	func = f;
	addr = cp->use(make_shared<JInteger>(i));
	pstream_addr = cp->use( make_shared<JFieldRef>("java/lang/System", "out", "Ljava/io/PrintStream;") );
	print_addr = cp->use(make_shared<JMethodRef>( "java/io/PrintStream", "println", "(I)V" ));
}

JavaNumber::~JavaNumber() {

}

void JavaNumber::push() {
	if (inst) {
		lhs->push();
		rhs->push();
		func->add(*inst);
	}
	else {
		func->pushConst(addr);
	}

}

shared_ptr<JavaNumber> JavaNumber::add(shared_ptr<JavaNumber> n) {
	shared_ptr<JavaNumber> jn = make_shared<JavaNumber>(func, pstream_addr, print_addr);

	jn->lhs = shared_from_this();
	jn->rhs = n;
	jn->inst = make_shared<JavaInstruction>(iadd);

	return jn;
}

void JavaNumber::print() {
	func->add(JavaInstruction(getstatic, make_shared<ConstReference>(pstream_addr, 2)));
	push();
	func->add(JavaInstruction(invoke, make_shared<ConstReference>(print_addr, 2)));
}

JavaString::JavaString(shared_ptr<JavaFunction> f, shared_ptr<ConstantPool> cp) {
	func = f;
	addr = cp->use(make_shared<JString>( "Hello" ));
	pstream_addr = cp->use( make_shared<JFieldRef>("java/lang/System", "out", "Ljava/io/PrintStream;") );
	print_addr = cp->use(make_shared<JMethodRef>( "java/io/PrintStream", "println", "(Ljava/lang/String;)V" ));
}

JavaString::~JavaString() {

}

void JavaString::print() {
	func->add(JavaInstruction(getstatic, make_shared<ConstReference>(pstream_addr, 2)));
	func->pushConst(addr);
	func->add(JavaInstruction(invoke, make_shared<ConstReference>(print_addr, 2)));
}

} /* namespace std */
