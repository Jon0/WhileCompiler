/*
 * X86Instruction.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include "X86Instruction.h"
#include "X86Reference.h"

namespace std {

X86Instruction::X86Instruction() {
	// TODO Auto-generated constructor stub

}

X86Instruction::~X86Instruction() {
	// TODO Auto-generated destructor stub
}

string InstrCode::width(shared_ptr<X86Reference> r) {
	return "l";
}

InstrPush::InstrPush(shared_ptr<X86Reference> r) {
	labelStr = r->place(8);
	type = "q";
}
InstrPush::~InstrPush() {}

string InstrPush::str() {
	return "\tpush"+type+"\t"+labelStr;
}

InstrMov::InstrMov(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t) {
	from = f->place(); to = t->place();
	type = "q";

	//if (t->)

	// TODO use register sizes
	if (from[0] == '$' || from[1] == 'e' || to[1] == 'e') type = "l";
}

} /* namespace std */
