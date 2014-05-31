/*
 * X86Function.cpp
 *
 *  Created on: 30/05/2014
 *      Author: remnanjona
 */

#include "X86Function.h"

namespace std {

X86Function::X86Function(string n, bool r, bool e) {
	name = n;
	has_return = r;
	external = e;

}

X86Function::~X86Function() {}

string X86Function::getName() {
	return name;
}

bool X86Function::hasReturn() {
	return has_return;
}

bool X86Function::isExternal() {
	return external;
}

} /* namespace std */
