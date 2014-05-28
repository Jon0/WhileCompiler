/*
 * X86Register.h
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#ifndef X86REGISTER_H_
#define X86REGISTER_H_

#include <string>

namespace std {

class X86Register {
public:
	X86Register(string);
	virtual ~X86Register();

	string place();

private:
	string name;
};

} /* namespace std */

#endif /* X86REGISTER_H_ */
