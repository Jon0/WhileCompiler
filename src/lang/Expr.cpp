/*
 * Expr.cpp
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#include "Expr.h"
#include "Func.h"

namespace std {

FuncCallExpr::FuncCallExpr( Token tok, shared_ptr<Func> f, vector<shared_ptr<Expr>> a ): Expr( tok, f->returnType() ) {
	func = f;
	args = a;
}

shared_ptr<Value> FuncCallExpr::eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {

	// push items in reverse order
	vector<shared_ptr<Value>> temp;
	for (shared_ptr<Expr> ep: args) {
		shared_ptr<Value> v = ep->eval(s, m);	// gets cloned later
		temp.push_back( v );
	}
	while ( !temp.empty() ) {
		s.push_back( temp.back() );
		temp.pop_back();
	}
	func->execute( s );

	// result return on stack top
	if ( func->returnType()->nameStr() != "void" ) {
		if (s.empty()) {
			throw runtime_error("empty stack: function expects return value");
		}
		shared_ptr<Value> result = s.back();
		s.pop_back();
		return result;
	}

	// need to have null returned
	return shared_ptr<Value>( new NullValue( shared_ptr<Type>( new NullType() ) ) );
}

void FuncCallExpr::typeCheck( CheckState &cs ) {
	for (shared_ptr<Expr> ep: args) {
		ep->typeCheck(cs);
	}
	// TODO check arg types match function
}

} /* namespace std */
