/*
 * X86Reference.h
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#ifndef X86REFERENCE_H_
#define X86REFERENCE_H_

#include <memory>
#include <string>

#include "X86Instruction.h"
#include "X86Program.h"
#include "X86Register.h"
#include "X86StackFrame.h"

namespace std {

class X86RegAddrRef;

/*
 * gives access to variables
 * vars can be stored both in registers or on stack/heap memory
 * also includes labeled constants, and basic constants
 * register access is faster
 */
class X86Reference {
public:
	X86Reference(int);
	virtual ~X86Reference();

	/*
	 * return assembly style string
	 */
	string place();
	virtual string place(int) = 0;	// force type width

	/*
	 * size of the reference
	 */
	int typeSize();

	/*
	 * a non addressed register
	 */
	virtual bool isRegister() {
		return false;
	}

private:
	int type_size;

};

/*
 * create reference to a labeled object
 */
class X86LabeledRef: public X86Reference, public enable_shared_from_this<X86LabeledRef> {
public:
	X86LabeledRef(string);
	virtual ~X86LabeledRef();

	virtual string place(int);

	string debug() {
		return "X86LabeledRef c:" + constant;
	}

private:
	string constant;
};

/*
 * create reference to a constant integer value
 * becomes $x in x86 assembly
 */
class X86ConstRef: public X86Reference, public enable_shared_from_this<X86ConstRef> {
public:
	X86ConstRef(long);
	virtual ~X86ConstRef();

	virtual string place(int);

	string debug() {
		return "X86LabeledRef c:" + to_string(constant);
	}

private:
	long constant;
};

/*
 *	%rax type locations
 *	parameters register, typesize
 */
class X86RegRef: public X86Reference, public enable_shared_from_this<X86RegRef> {
public:
	X86RegRef(shared_ptr<X86Register>, int);
	virtual ~X86RegRef();

	bool isLive();
	void free();

	virtual string place(int);

	/*
	 * creates another reference based on offset from this
	 */
	shared_ptr<X86RegAddrRef> index(int);		// paramter offset
	shared_ptr<X86RegAddrRef> index(int, int);	// parameters offset, size

	virtual bool isRegister() {
		return true;
	}

	shared_ptr<X86Register> getRegister();

private:
	shared_ptr<X86Register> reg;
	int reg_link_id;
	bool is_live;
};

/*
 *	-8(%rbp) type locations
 *	parameters register, offset, typesize
 */
class X86RegAddrRef: public X86Reference, public enable_shared_from_this<X86RegAddrRef> {
public:
	X86RegAddrRef(shared_ptr<X86Register>, int, int);
	virtual ~X86RegAddrRef();

	bool isLive();
	void free();

	virtual string place(int);

	/*
	 * creates another reference based on offset from this
	 */
	shared_ptr<X86RegAddrRef> index(int);		// paramter offset
	shared_ptr<X86RegAddrRef> index(int, int);	// parameters offset, size

	/*
	 * offset from register
	 */
	int getOffset();

	shared_ptr<X86Register> getRegister();

private:
	shared_ptr<X86Register> reg;
	int offset;
	int reg_link_id;
	bool is_live;
};




} /* namespace std */

#endif /* X86REFERENCE_H_ */
