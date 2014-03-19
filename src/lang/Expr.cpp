/*
 * Expr.cpp
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#include "Expr.h"
#include "Func.h"

namespace std {

FuncCallExpr::FuncCallExpr( shared_ptr<Func> f, vector<shared_ptr<Expr>> a ): Expr( f->returnType() ) {
	func = f;
	args = a;
}

shared_ptr<Value> FuncCallExpr::eval( Stack &s, VarMap m ) {

	// TODO check arg types

	// push items in reverse order
	vector<shared_ptr<Value>> temp;
	for (shared_ptr<Expr> ep: args) {
		temp.push_back( ep->eval(s, m) );
	}
	for (shared_ptr<Value> vp: temp) {
		s.push_back( vp );
	}

	func->execute( s );

	// result return on stack top
	if ( s.empty() ) { // m.count(var) == 0
		throw runtime_error("empty stack: function expects return value");
	}
	shared_ptr<Value> result = s.back();
	s.pop_back();
	return result;
}

} /* namespace std */
