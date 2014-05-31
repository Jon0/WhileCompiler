/*
 * X86Instruction.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include <iostream>

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

InstrPop::InstrPop(shared_ptr<X86Reference> r) {
	labelStr = r->place(8);
	type = "q";
}

InstrMov::InstrMov(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t) {
	from = f->place(); to = t->place();
	type = "q";

	if (from.length() > 10) {
		type = "absq";
	}

	// TODO use register sizes
	if (from[1] == 'e' || to[1] == 'e') type = "l";
}

InstrAdd::InstrAdd(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t) {
	from = f->place(); to = t->place();
	type = "q";

	// TODO use register sizes
	if (from[1] == 'e' || type[1] == 'e') type = "l";
}

InstrAdd::InstrAdd(int f, shared_ptr<X86Reference> t) {
	from = "$"+to_string(f); to = t->place();
	type = "q";

	// TODO use register sizes
	if (from[1] == 'e' || type[1] == 'e') type = "l";
}

InstrMul::InstrMul(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t) {
	from = f->place(); to = t->place();
	type = "q";

	// TODO use register sizes
	if (from[1] == 'e' || type[1] == 'e') type = "l";
}

InstrMul::InstrMul(int f, shared_ptr<X86Reference> t) {
	from = "$"+to_string(f); to = t->place();
	type = "q";

	// TODO use register sizes
	if (from[1] == 'e' || type[1] == 'e') type = "l";
}

} /* namespace std */
