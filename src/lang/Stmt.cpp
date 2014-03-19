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

void BlockStmt::execute( Stack &st, VarMap &m ) {
	for (shared_ptr<Stmt> &s: block) {
		s->execute( st, m );
	}
}

InitStmt::InitStmt(Var v): var(v) {
}

InitStmt::InitStmt(Var v, shared_ptr<Expr> e): var(v) {
	expr = e;
}

InitStmt::~InitStmt() {
}

void InitStmt::execute( Stack &s, VarMap &m ) {
	m.insert(VarMap::value_type(var, expr->eval( s, m )));
}

AssignStmt::AssignStmt(Var v, shared_ptr<Expr> e): var(v) {
	expr = e;
}

AssignStmt::~AssignStmt() {}

void AssignStmt::execute( Stack &s, VarMap &m ) {
	if ( m.find(var) == m.end() ) { // m.count(var) == 0
		throw runtime_error("variable not declared");
	}
	shared_ptr<Value> ev = expr->eval( s, m );
	m.erase(var);
	m.insert( VarMap::value_type(var, ev) );
}

ListAssignStmt::ListAssignStmt(Var v, shared_ptr<Expr> e, shared_ptr<Expr> i): var(v) {
	expr = e;
	ind = i;
}

ListAssignStmt::~ListAssignStmt() {}

void ListAssignStmt::execute( Stack &s, VarMap &m ) {
	shared_ptr<TypedValue<ValueList>> list = static_pointer_cast<TypedValue<ValueList>, Value>( m[var] );
	shared_ptr<TypedValue<int>> i = static_pointer_cast<TypedValue<int>, Value>( ind->eval( s, m ) );

	list->value().data()[ i->value() ] = expr->eval( s, m );
}

IfStmt::IfStmt(shared_ptr<Expr> e, shared_ptr<Stmt> b, shared_ptr<Stmt> a) {
	expr = e;
	body = b;
	alt = a;
}

IfStmt::~IfStmt() {
	// TODO Auto-generated destructor stub
}

void IfStmt::execute( Stack &s, VarMap &m ) {
	shared_ptr<TypedValue<bool>> a = static_pointer_cast<TypedValue<bool>, Value>( expr->eval( s, m ) );
	if ( a->value() ) {
		body->execute( s, m );
	}

	// check alt exists
	else if (alt) {
		alt->execute( s, m );
	}
}

PrintStmt::PrintStmt(shared_ptr<Expr> e) {
	expr = e;
}

PrintStmt::~PrintStmt() {
	// TODO Auto-generated destructor stub
}

void PrintStmt::execute( Stack &s, VarMap &m ) {
	cout << expr->eval( s, m )->asString() << endl;
}

ReturnStmt::ReturnStmt(shared_ptr<Expr> e) {
	expr = e;
}

ReturnStmt::~ReturnStmt() {
	// TODO Auto-generated destructor stub
}

void ReturnStmt::execute( Stack &s, VarMap &m ) {
	// put things on top of stack
	shared_ptr<Value> ev = expr->eval( s, m );
	s.push_back( ev );

	// TODO escape current block

}

} /* namespace std */
