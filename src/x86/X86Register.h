/*
 * X86Register.h
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#ifndef X86REGISTER_H_
#define X86REGISTER_H_

#include <memory>
#include <string>

namespace std {

class X86Instruction;
class X86Program;
class X86Reference;

class X86Register: public enable_shared_from_this<X86Register> {
public:
	X86Register(shared_ptr<X86Program>, string);
	virtual ~X86Register();

	string getName();

	bool inUse();
	void free();
	void setAsUsed();

	int getSize();
	void setSize(int);

	string place();
	string place(int);	// forced width

	void assign( shared_ptr<X86Reference> );
	void add( shared_ptr<X86Reference> );
	void sub( shared_ptr<X86Reference> );
	void multiply( shared_ptr<X86Reference> );
	void divide( shared_ptr<X86Reference> );
	void mod( shared_ptr<X86Reference> );
	void andBitwise( shared_ptr<X86Reference> );
	void orBitwise( shared_ptr<X86Reference> );
	void compare( shared_ptr<X86Reference> );
	void setFromFlags(string);

	// the thing thats currently assigned
	shared_ptr<X86Reference> ref();
	shared_ptr<X86Reference> ref(int);

	int getRefStackOffset();

private:
	shared_ptr<X86Program> program; // register is part of this program
	string name;
	int current_size;
	bool in_use;

	string sizeDesc();
};

} /* namespace std */

#endif /* X86REGISTER_H_ */
