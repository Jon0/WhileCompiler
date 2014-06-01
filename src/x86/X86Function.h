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
	/**
	 * parameters: function name, has return value, is external
	 */
	X86Function(string, bool, bool);
	virtual ~X86Function();

	string getName();
	bool hasReturn();
	bool isExternal();

private:
	string name;
	bool has_return;
	bool external;
};

} /* namespace std */

#endif /* X86FUNCTION_H_ */
