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
#include <unordered_set>

namespace std {

class X86Instruction;
class X86Program;
class X86Reference;
class X86RegRef;
class X86RegAddrRef;

enum instruction {
	add,
	sub
};

class X86Register: public enable_shared_from_this<X86Register> {
public:
	X86Register(shared_ptr<X86Program>, string);
	virtual ~X86Register();

	string getName();
	bool isMmx();

	bool inUse();
	void free(int);
	int use();

	int getSize();
	void setSize(int);

	string place();
	string place(int);	// forced width

	void assign( shared_ptr<X86Reference> );
	void assignAddrOf( shared_ptr<X86RegAddrRef> );

	// operations
	void add( shared_ptr<X86Reference> );
	void sub( shared_ptr<X86Reference> );
	void multiply( shared_ptr<X86Reference> );
	void divide( shared_ptr<X86Reference> );
	void mod( shared_ptr<X86Reference> );
	void andBitwise( shared_ptr<X86Reference> );
	void orBitwise( shared_ptr<X86Reference> );
	void compare( shared_ptr<X86Reference> );
	void setFromFlags(string);

	// make refernce to this register
	shared_ptr<X86RegRef> ref();
	shared_ptr<X86RegAddrRef> ref(int);

private:
	shared_ptr<X86Program> program; // register is part of this program
	string name;
	int current_size;
	bool is_mmx;

	// track usage of this register
	unordered_set<int> use_id;
	int next_id;

	string sizeDesc();
	shared_ptr<X86Reference> check( shared_ptr<X86Reference> );
};

} /* namespace std */

#endif /* X86REGISTER_H_ */
