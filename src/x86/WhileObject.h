/*
 * WhileObject.h
 *
 *  Created on: 30/05/2014
 *      Author: remnanjona
 */

#ifndef WHILEOBJECT_H_
#define WHILEOBJECT_H_

#include <memory>

#include "X86Program.h"
#include "X86StackFrame.h"

namespace std {

class Type;
class X86Program;
class X86Register;
class WhileObject;

/*
 * construct correct type - object, list or record
 */
shared_ptr<WhileObject> make_obj(shared_ptr<X86Program> p, shared_ptr<Type> wt);

/*
 * 16 byte object headers are too big for registers
 *
 * location defualt is base pointer addr
 * call putOnStack() to position on stack or
 * setLocation() for addr relative to another register
 */
class WhileObject {
public:
	WhileObject( shared_ptr<X86Program>, shared_ptr<Type> );
	virtual ~WhileObject();

	shared_ptr<Type> getType();

	void putOnStack();
	void pushStack();

	/**
	 * copy values into registers
	 * update memory values with next writeMem() call
	 */
	shared_ptr<X86Register> attachRegister();
	shared_ptr<X86Register> attachRegisterType();

	// as existing object
	void setLocation( shared_ptr<X86Register>, bool );
	void setLocation( mem_space, bool );

	// a new object
	virtual void initialise( shared_ptr<X86Reference>, bool );

	// ...
	void assign( shared_ptr<WhileObject>, bool );
	void assign( shared_ptr<X86Reference>, bool );
	void assignType( shared_ptr<Type>, bool );
	void writeMem();

	// direct references
	shared_ptr<X86Reference> tagDirect();
	shared_ptr<X86Reference> valueDirect();

	// address references
	shared_ptr<X86RegRef> addrRef();
	shared_ptr<X86Reference> tagRef();
	shared_ptr<X86Reference> valueRef();

	virtual string debug();

	static int getTypeSize(shared_ptr<Type> t);
	static int getTypeTag(shared_ptr<Type> t);

protected:
	shared_ptr<X86Program> program;

	// while type of this object
	shared_ptr<Type> w_type;

	// memory space
	mem_space space;
	bool initialised;

	// values
	shared_ptr<X86Reference> ref;
	shared_ptr<X86Reference> type;

	// make sure the correct register type is used,
	// require mmx registers for doubles
	void attachRegister(shared_ptr<X86Register>);
	void attachRegisterType(shared_ptr<X86Register>);

};

class WhileList: public WhileObject {
public:
	WhileList( shared_ptr<X86Program>, shared_ptr<Type> );
	virtual ~WhileList();

	virtual void initialise(shared_ptr<X86Reference>, bool write);

	virtual string debug();

	void length(shared_ptr<WhileObject>);
	void get(shared_ptr<WhileObject>, shared_ptr<X86Reference>);

	template<class T> shared_ptr<T> get( shared_ptr<X86Reference> ref ) {
		shared_ptr<T> wo = make_shared<T>(program, inner_type);
		get( wo, ref );
		return wo;
	}

	shared_ptr<WhileObject> get( shared_ptr<X86Reference> ref ) {
		shared_ptr<WhileObject> wo = make_obj(program, inner_type);
		get( wo, ref );
		return wo;
	}

private:
	shared_ptr<Type> inner_type;
};

class WhileRecord: public WhileObject {
public:
	WhileRecord( shared_ptr<X86Program>, shared_ptr<Type> );
	virtual ~WhileRecord();

	virtual void initialise(shared_ptr<X86Reference>, bool write);
};

} /* namespace std */

#endif /* WHILEOBJECT_H_ */
