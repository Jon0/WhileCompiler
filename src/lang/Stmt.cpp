/*
 * Stmt.cpp
 *
 *  Created on: 16/03/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "Expr.h"
#include "Stmt.h"
#include "SyntaxElem.h"

namespace std {

BlockStmt::BlockStmt( vector<shared_ptr<Stmt>> b ) {
	block = b;
}

BlockStmt::~BlockStmt() {
}

StmtStatus BlockStmt::execute( Stack &st, VarMap &m ) {
	for (shared_ptr<Stmt> &s: block) {
		StmtStatus ss = s->execute( st, m );
		if (ss.isReturn) {
			return {true, false}; 	// propogate back
		}
		else if (ss.isBreak) {
			return {false, false};
		}
	}
	return {false, false};
}

void BlockStmt::typeCheck( CheckState &cks ) {
	for (shared_ptr<Stmt> &s: block) {
		s->typeCheck(cks);
	}
}

InitStmt::InitStmt(Var v): var(v) {
	expr = NULL;
}

InitStmt::InitStmt(Var v, shared_ptr<Expr> e): var(v) {
	expr = e;
}

InitStmt::~InitStmt() {
}

StmtStatus InitStmt::execute( Stack &s, VarMap &m ) {
	if (expr) {
		m.insert(VarMap::value_type( var, expr->eval( s, m )->clone( var.type() ) ));	// always clone rhs on assignment
	}
	else {
		m.insert(VarMap::value_type(var, shared_ptr<Value>( new TypedValue<int>(var.type(), 0) )));
	}

	return {false, false};
}

void InitStmt::typeCheck( CheckState &cs ) {
	if (expr) {
		expr->typeCheck(cs);
		if (!var.type()->contains(*expr->getType())) {
			throw TokenException(expr->getTokens(), "expected type "+var.type()->aliasStr()+", found "+expr->getType()->aliasStr());
		}
	}
	AssignState as{expr != NULL, var.type()};
	cs.assigned.insert( map<string, AssignState>::value_type(var.name(), as) );
}

AssignStmt::AssignStmt(shared_ptr<Expr> l, shared_ptr<Expr> r) {
	lhs = l;
	rhs = r;
}

AssignStmt::~AssignStmt() {}

StmtStatus AssignStmt::execute( Stack &s, VarMap &m ) {
	shared_ptr<Value> *assignable = NULL;

	lhs->eval( s, m, &assignable );
	if (!assignable) {
		throw runtime_error("interpreter error: lhs not assignable");
	}
	shared_ptr<Value> ev = rhs->eval( s, m )->clone( lhs->getType() ); // always clone rhs on assignment
	*assignable = ev;

	return {false, false};
}

void AssignStmt::typeCheck( CheckState &cs ) {
	shared_ptr<Var> var = lhs->assignable(cs);
	if (!var) {
		throw runtime_error("lhs not assignable");
	}
	rhs->typeCheck(cs);
	if (!lhs->getType()->contains( *rhs->getType() )) {
		throw TokenException(rhs->getTokens(),
				"expected type " + lhs->getType()->nameStr() + ", found "
						+ rhs->getType()->nameStr());
	}
	map<string, AssignState>::iterator it = cs.assigned.find( var->name() );
	(*it).second.defAssign = true;
}

IfStmt::IfStmt(shared_ptr<Expr> e, shared_ptr<Stmt> b, shared_ptr<Stmt> a) {
	expr = e;
	body = b;
	alt = a;
}

IfStmt::~IfStmt() {}

StmtStatus IfStmt::execute( Stack &s, VarMap &m ) {
	shared_ptr<TypedValue<bool>> a = static_pointer_cast<TypedValue<bool>, Value>( expr->eval( s, m ) );
	if ( a->value() ) {
		StmtStatus ss = body->execute( s, m );
		if (ss.isReturn) {
			return {true, false}; 	// propogate back
		}
		else if (ss.isBreak) {
			return {false, false};
		}
	}

	// check alt exists
	else if (alt) {
		StmtStatus ss = alt->execute( s, m );
		if (ss.isReturn) {
			return {true, false}; 	// propogate back
		}
		else if (ss.isBreak) {
			return {false, false};
		}
	}
	return {false, false};
}

void IfStmt::typeCheck( CheckState &cs ) {
	expr->typeCheck(cs);

	// check is expr boolean
	boolCheck(expr);

	CheckState csbody = cs;
	body->typeCheck(csbody);

	CheckState csalt = cs;
	if (alt) {
		alt->typeCheck(csalt);
	}

	// find intersection
	for (map<string, AssignState>::value_type &as: cs.assigned) {
		map<string, AssignState>::iterator bit = csbody.assigned.find(as.first);
		map<string, AssignState>::iterator ait = csalt.assigned.find(as.first);

		as.second.defAssign |= (csbody.returned || (*bit).second.defAssign)
				&& (csalt.returned || (*ait).second.defAssign);

		// TODO intersection of types

	}

	cs.returned |= csbody.returned || csalt.returned;
}

WhileStmt::WhileStmt(shared_ptr<Expr> e, shared_ptr<Stmt> b) {
	expr = e;
	body = b;
}

WhileStmt::~WhileStmt() {}

StmtStatus WhileStmt::execute( Stack &s, VarMap &m ) {
	shared_ptr<TypedValue<bool>> a = static_pointer_cast<TypedValue<bool>, Value>( expr->eval( s, m ) );
	while ( a->value() ) {
		StmtStatus ss = body->execute( s, m );
		if (ss.isReturn) {
			return {true, false}; 	// propogate back
		}
		else if (ss.isBreak) {
			return {false, false};
		}

		// reevaluate loop condition
		a = static_pointer_cast<TypedValue<bool>, Value>( expr->eval( s, m ) );
	}
	return {false, false};
}

void WhileStmt::typeCheck( CheckState &cs ) {
	expr->typeCheck(cs);

	// keep internal state seperated
	CheckState internal = cs;
	body->typeCheck(internal);

	// check is expr boolean
	boolCheck(expr);
}


ForStmt::ForStmt( shared_ptr<Stmt> i, shared_ptr<Expr> c, shared_ptr<Stmt> u, shared_ptr<Stmt> b ) {
	init = i;
	expr = c;
	inc = u;
	body = b;
}

ForStmt::~ForStmt() {

}

bool ForStmt::checkCond( Stack &s, VarMap &m ) {
	if (expr) {
		shared_ptr<TypedValue<bool>> a = static_pointer_cast<TypedValue<bool>, Value>(expr->eval(s, m));
		return a->value();
	}
	else {
		return true; // when no condition eg. "for(;;){}"
	}
}

StmtStatus ForStmt::execute( Stack &s, VarMap &m ) {
	// init
	if (init) init->execute(s, m);

	while ( checkCond( s, m ) ) {
		StmtStatus ss = body->execute( s, m );
		if (ss.isReturn) {
			return {true, false}; 	// propogate back
		}
		else if (ss.isBreak) {
			return {false, false};
		}

		// inc and reevaluate loop condition
		if (inc) inc->execute(s, m);
	}
	return {false, false};
}

void ForStmt::typeCheck( CheckState &cs ) {
	init->typeCheck(cs);
	expr->typeCheck(cs);
	inc->typeCheck(cs);

	CheckState internal = cs;
	body->typeCheck(internal);

	// check is expr boolean
	boolCheck(expr);
}

PrintStmt::PrintStmt(shared_ptr<Expr> e) {
	expr = e;
}

PrintStmt::~PrintStmt() {}

StmtStatus PrintStmt::execute( Stack &s, VarMap &m ) {
	cout << expr->eval( s, m )->asString() << endl;
	return {false, false};
}

void PrintStmt::typeCheck( CheckState &cs ) {
	expr->typeCheck(cs);
}

EvalStmt::EvalStmt(shared_ptr<Expr> e) {
	expr = e;
}

EvalStmt::~EvalStmt() {}

StmtStatus EvalStmt::execute( Stack &s, VarMap &m ) {
	expr->eval( s, m );
	return {false, false};
}

void EvalStmt::typeCheck( CheckState &cs ) {
	expr->typeCheck(cs);
}

ReturnStmt::ReturnStmt() {
	expr = NULL;
}

ReturnStmt::ReturnStmt(shared_ptr<Expr> e) {
	expr = e;
}

ReturnStmt::~ReturnStmt() {}

StmtStatus ReturnStmt::execute( Stack &s, VarMap &m ) {
	// put things on top of stack

	if (expr) {
		shared_ptr<Value> ev = expr->eval( s, m );
		s.push_back( ev );
	}

	// escape current block
	return {true, false};

}

void ReturnStmt::typeCheck( CheckState &s ) {
	if (expr) {
		expr->typeCheck(s);
		if (!s.to_return->contains(*expr->getType())) {
			throw TokenException(expr->getTokens(),
					"expected type " + s.to_return->aliasStr() + ", found "
							+ expr->getType()->aliasStr());
		}
	}
	else {
		if (s.to_return->nameStr() != "void") {
			throw runtime_error("return value required");
		}
	}
	s.returned = true;
}

BreakStmt::BreakStmt() {}

BreakStmt::~BreakStmt() {}

StmtStatus BreakStmt::execute( Stack &s, VarMap &m ) {
	// escape current block
	return {false, true};
}

void BreakStmt::typeCheck( CheckState & ) {
	// TODO unreachable code
}

SwitchStmt::SwitchStmt( shared_ptr<Expr> e, map<shared_ptr<Expr>, shared_ptr<Stmt>> l, shared_ptr<Stmt> d ) {
	expr = e;
	list = l;
	def_stmt = d;
}

SwitchStmt::~SwitchStmt() {}

StmtStatus SwitchStmt::execute( Stack &s, VarMap &m ) {
	shared_ptr<Value> ev = expr->eval( s, m );
	for ( map<shared_ptr<Expr>, shared_ptr<Stmt>>::value_type ex: list ) {
		shared_ptr<Value> cv = ex.first->eval( s, m );

		if (*cv == *ev) {
			StmtStatus ss = ex.second->execute( s, m );
			return {ss.isReturn, false}; 	// propogate back
		}
	}

	if (def_stmt) {
		StmtStatus ss = def_stmt->execute( s, m );
		return {ss.isReturn, false}; 	// propogate back
	}
	return {false, false};
}

void SwitchStmt::typeCheck( CheckState &cs ) {
	vector<CheckState> states;
	CheckState ds = cs; // for defualt

	for ( map<shared_ptr<Expr>, shared_ptr<Stmt>>::value_type ex: list ) {
		ex.first->typeCheck(cs);
		states.push_back(CheckState(cs));
		ex.second->typeCheck(states.back());
	}
	if (def_stmt) {
		def_stmt->typeCheck(ds);
	}

	// find intersection
	for (map<string, AssignState>::value_type &as: cs.assigned) {
		bool assigned = true;
		for (CheckState &st: states) {
			map<string, AssignState>::iterator it = st.assigned.find(as.first);
			assigned &= (st.returned || (*it).second.defAssign);
		}
		if (def_stmt) {
			map<string, AssignState>::iterator it = ds.assigned.find(as.first);
			assigned &= (ds.returned || (*it).second.defAssign);
		}
		else {
			assigned = false; // cannot be sure of assignment without default
		}
		as.second.defAssign |= assigned;

		// TODO intersection of types
	}
}



} /* namespace std */
