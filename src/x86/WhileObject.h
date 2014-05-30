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
 */
class WhileObject {
public:
	WhileObject( shared_ptr<X86Program> );
	virtual ~WhileObject();

	void putOnStack();

	void initialise( shared_ptr<X86Register> );
	void initialise( shared_ptr<X86Reference> v, int type );
	virtual void initialise(shared_ptr<X86Reference>);

	void free();

	shared_ptr<X86Reference> addrRef();
	shared_ptr<X86Reference> tagRef();
	shared_ptr<X86Reference> valueRef();

protected:
	shared_ptr<X86Program> program;
	shared_ptr<X86Register> base;
	StackSpace space;
};

class WhileList: public WhileObject {
public:
	WhileList( shared_ptr<X86Program> );
	virtual ~WhileList();

	virtual void initialise(shared_ptr<X86Reference>);

	void length(shared_ptr<WhileObject>);
	void get(shared_ptr<WhileObject>, shared_ptr<X86Reference>);

	template<class T> shared_ptr<T> get( shared_ptr<X86Reference> ref ) {
		shared_ptr<T> wo = make_shared<T>(program);
		get( wo, ref );
		return wo;
	}
};

} /* namespace std */

#endif /* WHILEOBJECT_H_ */
