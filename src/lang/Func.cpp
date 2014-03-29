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
}

Func::~Func() {}

string Func::name() const {
	return name_str;
}

shared_ptr<Type> Func::returnType() const {
	return r_type;
}

void Func::execute( Stack &stack ) {
	VarMap vars;

	// TODO check arg types
	for (Var &v: args) {
		shared_ptr<Value> ev = stack.back()->clone( v.type() ); // clone as type of arg
		stack.pop_back();
		vars.insert( VarMap::value_type(v, ev) );
	}

	stmts->execute( stack, vars );
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
}

} /* namespace std */
