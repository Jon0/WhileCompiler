/*
 * Expr.cpp
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#include "Expr.h"
#include "Func.h"

namespace std {

/*
 * throw error if expr is not boolean
 */
void boolCheck(shared_ptr<Expr> expr) {
	if (expr->getType()->nameStr() != "bool") {
		throw TokenException(expr->getTokens(),
				"expected type bool, found "
						+ expr->getType()->aliasStr());
	}
}

shared_ptr<Type> findListType(vector<shared_ptr<Expr>> e) {
	if (e.empty()) return shared_ptr<Type>(new ListType());

	vector<shared_ptr<Type>> inner_type;

	for (shared_ptr<Expr> ex: e) {
		bool found = false;
		for (shared_ptr<Type> ty: inner_type) {
			if (ty->contains(*ex->getType())) {
				found = true;
				break;
			}
		}
		if (!found) {
			inner_type.push_back(ex->getType());
		}
	}

	if (inner_type.size() == 1) {
		return shared_ptr<Type>(new ListType(inner_type[0]));
	}
	else {
		return shared_ptr<Type>(new ListType(shared_ptr<Type>(new UnionType(inner_type))));
	}
}

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
	if (func->numArgs() != args.size()) {
		throw TokenException(getTokens(), "Incorrect number of arguments");
	}

	int i = 0;
	for (shared_ptr<Expr> ep: args) {
		ep->typeCheck(cs);
		if ( !func->argType(i)->contains( *ep->getType() )) {
			throw TokenException(ep->getTokens(), "expected type "+func->argType(i)->aliasStr()+", found "+ep->getType()->aliasStr());
		}

		i++;
	}
	// TODO check arg types match function
}

} /* namespace std */
