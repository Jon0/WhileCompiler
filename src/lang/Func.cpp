/*
 * Func.cpp
 *
 *  Created on: 16/03/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "Func.h"

namespace std {

Func::Func( string name, shared_ptr<Type> t, vector<Var> a, shared_ptr<Stmt> s ): r_type(t) {
	name_str = name;
	args = a;
	stmts = s;
}

Func::~Func() {
	// TODO Auto-generated destructor stub
}

string Func::name() const {
	return name_str;
}

shared_ptr<Type> Func::returnType() {
	return r_type;
}

void Func::execute( Stack &stack ) {
	VarMap vars;

	// TODO check arg types
	for (Var &v: args) {
		shared_ptr<Value> ev = stack.back();
		stack.pop_back();
		vars.insert( VarMap::value_type(v, ev) );
	}

	stmts->execute( stack, vars );
}

} /* namespace std */
