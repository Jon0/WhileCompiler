/*
 * JavaInstruction.cpp
 *
 *  Created on: 19/06/2014
 *      Author: asdf
 */

#include "JavaInstruction.h"

namespace std {

JavaInstruction::JavaInstruction(IName n) {
	name = n;
	args = IArgs{};
}

JavaInstruction::JavaInstruction(IName n, shared_ptr<JavaReference> ref) {
	name = n;
	args = IArgs{ref};
}

JavaInstruction::JavaInstruction(IName n, IArgs a) {
	name = n;
	args = a;
}

JavaInstruction::~JavaInstruction() {
	// TODO Auto-generated destructor stub
}

short JavaInstruction::size() {
	short total = 1;
	for (auto ap: args) {
		total += ap->size();
	}
	return total;
}

bytecode JavaInstruction::toBytecode() {
	bytecode b;
	write_u1(b, name);
	for (shared_ptr<JavaReference> jr: args) {
		write_list(b, jr->toByteCode());
	}
	return b;
}

} /* namespace std */
