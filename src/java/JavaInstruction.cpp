/*
 * JavaInstruction.cpp
 *
 *  Created on: 19/06/2014
 *      Author: asdf
 */

#include "JavaInstruction.h"

namespace std {

JavaInstruction::JavaInstruction(IName n, IArgs a) {
	name = n;
	args = a;
}

JavaInstruction::~JavaInstruction() {
	// TODO Auto-generated destructor stub
}

vector<char> JavaInstruction::toBytecode() {
	vector<char> b;

	switch (name) {
	case ifeq:
		b.push_back(0x99);
		break;
	}

	return b;
}

} /* namespace std */
