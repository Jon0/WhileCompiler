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
class X86Reference;

class X86Register: public enable_shared_from_this<X86Register> {
public:
	X86Register(string);
	virtual ~X86Register();

	string place();

	void assign(shared_ptr<X86Reference>);

private:
	string sizeDesc();

	string name;
	shared_ptr<X86Reference> ref; // the thing thats currently assigned
};

} /* namespace std */

#endif /* X86REGISTER_H_ */
