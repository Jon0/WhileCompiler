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

InitStmt::InitStmt(Var v): var(v) {
}

InitStmt::InitStmt(Var v, shared_ptr<Expr> e): var(v) {
	expr = e;
}

InitStmt::~InitStmt() {
}

StmtStatus InitStmt::execute( Stack &s, VarMap &m ) {
	if (expr) {
		m.insert(VarMap::value_type(var, expr->eval( s, m )));
	}
	else {
		m.insert(VarMap::value_type(var, shared_ptr<Value>( new TypedValue<int>(var.type(), 0) )));
	}

	return {false, false};
}

AssignStmt::AssignStmt(Var v, shared_ptr<Expr> e): var(v) {
	expr = e;
}

AssignStmt::~AssignStmt() {}

StmtStatus AssignStmt::execute( Stack &s, VarMap &m ) {
	if ( m.find(var) == m.end() ) { // m.count(var) == 0
		throw runtime_error("variable not declared");
	}
	shared_ptr<Value> ev = expr->eval( s, m );
	m.erase(var);
	m.insert( VarMap::value_type(var, ev) );
	return {false, false};
}

ListAssignStmt::ListAssignStmt(Var v, shared_ptr<Expr> e, shared_ptr<Expr> i): var(v) {
	expr = e;
	ind = i;
}

ListAssignStmt::~ListAssignStmt() {}

StmtStatus ListAssignStmt::execute( Stack &s, VarMap &m ) {
	shared_ptr<TypedValue<ValueList>> list = static_pointer_cast<TypedValue<ValueList>, Value>( m[var] );
	shared_ptr<TypedValue<int>> i = static_pointer_cast<TypedValue<int>, Value>( ind->eval( s, m ) );

	list->value().data()[ i->value() ] = expr->eval( s, m );
	return {false, false};
}

RecordAssignStmt::RecordAssignStmt(Var v, shared_ptr<Expr> e, string m): var(v) {
	expr = e;
	member = m;
}

RecordAssignStmt::~RecordAssignStmt() {}

StmtStatus RecordAssignStmt::execute( Stack &s, VarMap &m ) {
	shared_ptr<TypedValue<ValueRecord>> list = static_pointer_cast<TypedValue<ValueRecord>, Value>( m[var] );


	list->value()[ member ] = expr->eval( s, m );
	return {false, false};
}

IfStmt::IfStmt(shared_ptr<Expr> e, shared_ptr<Stmt> b, shared_ptr<Stmt> a) {
	expr = e;
	body = b;
	alt = a;
}

IfStmt::~IfStmt() {
	// TODO Auto-generated destructor stub
}

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
		StmtStatus ss =alt->execute( s, m );
		if (ss.isReturn) {
			return {true, false}; 	// propogate back
		}
		else if (ss.isBreak) {
			return {false, false};
		}
	}
	return {false, false};
}

WhileStmt::WhileStmt(shared_ptr<Expr> e, shared_ptr<Stmt> b) {
	expr = e;
	body = b;
}

WhileStmt::~WhileStmt() {
	// TODO Auto-generated destructor stub
}

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

ForStmt::ForStmt( shared_ptr<Stmt> i, shared_ptr<Expr> c, shared_ptr<Stmt> u, shared_ptr<Stmt> b ) {
	init = i;
	expr = c;
	inc = u;
	body = b;
}

ForStmt::~ForStmt() {

}

StmtStatus ForStmt::execute( Stack &s, VarMap &m ) {
	// init
	if (init) init->execute(s, m);

	shared_ptr<TypedValue<bool>> a = static_pointer_cast<TypedValue<bool>, Value>( expr->eval( s, m ) );	// TODO when no condition eg. "for(;;){}"
	while ( a->value() ) {
		StmtStatus ss = body->execute( s, m );
		if (ss.isReturn) {
			return {true, false}; 	// propogate back
		}
		else if (ss.isBreak) {
			return {false, false};
		}

		// inc and reevaluate loop condition
		if (inc) inc->execute(s, m);
		a = static_pointer_cast<TypedValue<bool>, Value>( expr->eval( s, m ) );
	}
	return {false, false};
}

PrintStmt::PrintStmt(shared_ptr<Expr> e) {
	expr = e;
}

PrintStmt::~PrintStmt() {
	// TODO Auto-generated destructor stub
}

StmtStatus PrintStmt::execute( Stack &s, VarMap &m ) {
	cout << expr->eval( s, m )->asString() << endl;
	return {false, false};
}

EvalStmt::EvalStmt(shared_ptr<Expr> e) {
	expr = e;
}

EvalStmt::~EvalStmt() {
	// TODO Auto-generated destructor stub
}

StmtStatus EvalStmt::execute( Stack &s, VarMap &m ) {
	expr->eval( s, m );
	return {false, false};
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

BreakStmt::BreakStmt() {}

BreakStmt::~BreakStmt() {}

StmtStatus BreakStmt::execute( Stack &s, VarMap &m ) {
	// escape current block
	return {false, true};

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
			return {ss.isReturn}; 	// propogate back
		}
	}

	if (def_stmt) {
		StmtStatus ss = def_stmt->execute( s, m );
		return {ss.isReturn}; 	// propogate back
	}
	return {false};
}



} /* namespace std */
