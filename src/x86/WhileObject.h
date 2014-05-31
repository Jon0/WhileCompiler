/*
 * WhileObject.h
 *
 *  Created on: 30/05/2014
 *      Author: remnanjona
 */

#ifndef WHILEOBJECT_H_
#define WHILEOBJECT_H_

#include <memory>

#include "X86StackFrame.h"

namespace std {

class X86Program;
class X86Register;

/*
 * 16 byte object headers are too big for registers
 *
 * location defualt is base pointer addr
 * call putOnStack() to position on stack or
 * setLocation() for addr relative to another register
 */
class WhileObject {
public:
	WhileObject( shared_ptr<X86Program> );
	virtual ~WhileObject();

	void putOnStack();
	void pushStack();

	//
	shared_ptr<X86Register> attachRegister();

	// as existing object
	void setLocation( shared_ptr<X86Register> );
	void setLocation( shared_ptr<X86Register>,StackSpace );

	// a new object
	void initialise( shared_ptr<X86Reference> v, int type, bool write );
	virtual void initialise(shared_ptr<X86Reference>, bool write);

	// ...
	void assign( shared_ptr<WhileObject>, bool );
	void writeMem();
	void free();


	// direct references
	shared_ptr<X86Reference> tagDirect();
	shared_ptr<X86Reference> valueDirect();

	// address references
	shared_ptr<X86Reference> addrRef();
	shared_ptr<X86Reference> tagRef();
	shared_ptr<X86Reference> valueRef();

protected:
	shared_ptr<X86Program> program;
	shared_ptr<X86Register> base;
	StackSpace space;

	// values
	shared_ptr<X86Reference> ref;
	shared_ptr<X86Reference> type;

	bool initialised;
};

class WhileList: public WhileObject {
public:
	WhileList( shared_ptr<X86Program> );
	virtual ~WhileList();

	virtual void initialise(shared_ptr<X86Reference>, bool write);

	void length(shared_ptr<WhileObject>);
	void get(shared_ptr<WhileObject>, shared_ptr<X86Reference>);

	template<class T> shared_ptr<T> get( shared_ptr<X86Reference> ref ) {
		shared_ptr<T> wo = make_shared<T>(program);
		get( wo, ref );
		return wo;
	}
};

class WhileRecord: public WhileObject {
public:
	WhileRecord( shared_ptr<X86Program> );
	virtual ~WhileRecord();

	virtual void initialise(shared_ptr<X86Reference>, bool write);
};

} /* namespace std */

#endif /* WHILEOBJECT_H_ */
