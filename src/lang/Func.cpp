/*
 * Func.cpp
 *
 *  Created on: 16/03/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "Func.h"
#include "Stmt.h"

namespace std {

Func::Func( string name, shared_ptr<Type> t, vector<Var> a, shared_ptr<Stmt> s ): r_type(t) {
	name_str = name;
	args = a;
	stmts = s;

	addChild(s);
}

Func::~Func() {}

string Func::name() const {
	return name_str;
}

int Func::numArgs() const {
	return args.size();
}

shared_ptr<Type> Func::argType(int i) const {
	if (0 <= i && i < args.size()) {
		return args[i].type();
	}
	else {
		throw runtime_error("function arg index out of range");
	}

}

shared_ptr<Type> Func::returnType() const {
	return r_type;
}

vector<Var> Func::getArgs() const {
	return args;
}

shared_ptr<Stmt> Func::getStmt() const {
	return stmts;
}

void Func::execute( Stack &stack ) {

}

void Func::typeCheck() {
	CheckState cs;
	cs.to_return = r_type;
	cs.returned = false;

	for (Var &v: args) {
		AssignState as;
		as.defAssign = true;
		as.type = v.type();
		cs.assigned.insert( map<string, AssignState>::value_type(v.name(), as) );
	}

	stmts->typeCheck(cs);

	if ( r_type->nameStr() != "void" && !cs.returned ) {
		throw runtime_error("non void function requires return value");
	}
}

void Func::visit(shared_ptr<SyntaxVisitor> v) {
	return v->accept( shared_from_this() );
}

} /* namespace std */
