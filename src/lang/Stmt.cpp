/*
 * Stmt.cpp
 *
 *  Created on: 16/03/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "Stmt.h"

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
			return {true}; 	// propogate back
		}
	}
	return {false};
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
	return {false};
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
	return {false};
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
	return {false};
}

RecordAssignStmt::RecordAssignStmt(Var v, shared_ptr<Expr> e, string m): var(v) {
	expr = e;
	member = m;
}

RecordAssignStmt::~RecordAssignStmt() {}

StmtStatus RecordAssignStmt::execute( Stack &s, VarMap &m ) {
	shared_ptr<TypedValue<ValueRecord>> list = static_pointer_cast<TypedValue<ValueRecord>, Value>( m[var] );


	list->value()[ member ] = expr->eval( s, m );
	return {false};
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
			return {true}; 	// propogate back
		}
	}

	// check alt exists
	else if (alt) {
		StmtStatus ss =alt->execute( s, m );
		if (ss.isReturn) {
			return {true}; 	// propogate back
		}
	}
	return {false};
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
			return {true}; 	// propogate back
		}
	}
	return {false};
}

PrintStmt::PrintStmt(shared_ptr<Expr> e) {
	expr = e;
}

PrintStmt::~PrintStmt() {
	// TODO Auto-generated destructor stub
}

StmtStatus PrintStmt::execute( Stack &s, VarMap &m ) {
	cout << expr->eval( s, m )->asString() << endl;
	return {false};
}

ReturnStmt::ReturnStmt(shared_ptr<Expr> e) {
	expr = e;
}

ReturnStmt::~ReturnStmt() {
	// TODO Auto-generated destructor stub
}

StmtStatus ReturnStmt::execute( Stack &s, VarMap &m ) {
	// put things on top of stack
	shared_ptr<Value> ev = expr->eval( s, m );
	s.push_back( ev );

	// escape current block
	return {true};

}

} /* namespace std */
