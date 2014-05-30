/*
 * X86Function.h
 *
 *  Created on: 30/05/2014
 *      Author: remnanjona
 */

#ifndef X86FUNCTION_H_
#define X86FUNCTION_H_

#include <string>

namespace std {

class X86Function {
public:
	X86Function(string, bool);
	virtual ~X86Function();

	string getName();
	bool isExternal();

private:
	string name;
	bool external;
};

} /* namespace std */

#endif /* X86FUNCTION_H_ */
