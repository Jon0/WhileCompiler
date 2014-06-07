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
	int toSize = t->typeSize();
	from = f->place(toSize); to = t->place();
	type = "q";
	ext = "";

	// TODO use register sizes
	if (from[1] == 'e' || to[1] == 'e') type = "l";
}

InstrMov::InstrMov(string p, shared_ptr<X86Reference> f, shared_ptr<X86Reference> t) {
	// f use defualt size
	// zero padding could be used
	from = f->place(); to = t->place();
	type = "q";
	ext = p;

	// TODO use register sizes
	if (from[1] == 'e' || to[1] == 'e') type = "l";
}

InstrLea::InstrLea(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t) {
	int toSize = t->typeSize();
	from = f->place(toSize); to = t->place();
	type = "q";

	// TODO use register sizes
	if (from[1] == 'e' || to[1] == 'e') type = "l";
}

InstrAdd::InstrAdd(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t) {
	int toSize = t->typeSize();
	from = f->place(toSize); to = t->place();
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

InstrSub::InstrSub(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t) {
	int toSize = t->typeSize();
	from = f->place(toSize); to = t->place();
	type = "q";

	// TODO use register sizes
	if (from[1] == 'e' || type[1] == 'e') type = "l";
}

InstrSub::InstrSub(int f, shared_ptr<X86Reference> t) {
	from = "$"+to_string(f); to = t->place();
	type = "q";

	// TODO use register sizes
	if (from[1] == 'e' || type[1] == 'e') type = "l";
}

InstrMul::InstrMul(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t) {
	int toSize = t->typeSize();
	from = f->place(toSize); to = t->place();
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

InstrDiv::InstrDiv(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t) {
	int toSize = t->typeSize();
	from = f->place(toSize); to = t->place();
	type = "q";

	// TODO use register sizes
	if (from[1] == 'e' || type[1] == 'e') type = "l";
}

InstrDiv::InstrDiv(int f, shared_ptr<X86Reference> t) {
	from = "$"+to_string(f); to = t->place();
	type = "q";

	// TODO use register sizes
	if (from[1] == 'e' || type[1] == 'e') type = "l";
}

} /* namespace std */
