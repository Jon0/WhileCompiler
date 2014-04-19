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

void BlockStmt::typeCheck( CheckState &cks ) {
	for (shared_ptr<Stmt> &s: block) {
		s->typeCheck(cks);
	}
}

void BlockStmt::visit(shared_ptr<SyntaxVisitor> v) {
	v->accept( shared_from_this() );
}

vector<shared_ptr<Stmt>> BlockStmt::getStmt() {
	return block;
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

void InitStmt::visit(shared_ptr<SyntaxVisitor> v) {
	v->accept( shared_from_this() );
}

bool InitStmt::hasInit() {
	return expr? true: false;
}

Var InitStmt::getVar() {
	return var;
}

shared_ptr<Expr> InitStmt::getExpr() {
	return expr;
}

AssignStmt::AssignStmt(shared_ptr<Expr> l, shared_ptr<Expr> r) {
	lhs = l;
	rhs = r;
}

AssignStmt::~AssignStmt() {}

StmtStatus AssignStmt::execute( Stack &s, VarMap &m ) {

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

shared_ptr<Expr> AssignStmt::getLHS() { return lhs; }
shared_ptr<Expr> AssignStmt::getRHS() { return rhs; }

void AssignStmt::visit(shared_ptr<SyntaxVisitor> v) {
	v->accept( shared_from_this() );
}

IfStmt::IfStmt(shared_ptr<Expr> e, shared_ptr<Stmt> b, shared_ptr<Stmt> a) {
	expr = e;
	body = b;
	alt = a;
}

IfStmt::~IfStmt() {}

StmtStatus IfStmt::execute( Stack &s, VarMap &m ) {

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

void IfStmt::visit(shared_ptr<SyntaxVisitor> v) {
	v->accept( shared_from_this() );
}

shared_ptr<Expr> IfStmt::getExpr() {
	return expr;
}

shared_ptr<Stmt> IfStmt::getBody() {
	return body;
}

shared_ptr<Stmt> IfStmt::getAlt() {
	return alt;
}

bool IfStmt::hasAlt() {
	return alt != NULL;
}

WhileStmt::WhileStmt(shared_ptr<Expr> e, shared_ptr<Stmt> b) {
	expr = e;
	body = b;
}

WhileStmt::~WhileStmt() {}

StmtStatus WhileStmt::execute( Stack &s, VarMap &m ) {

}


void WhileStmt::typeCheck( CheckState &cs ) {
	expr->typeCheck(cs);

	// keep internal state seperated
	CheckState internal = cs;
	body->typeCheck(internal);

	// check is expr boolean
	boolCheck(expr);
}

void WhileStmt::visit(shared_ptr<SyntaxVisitor> v) {
	v->accept( shared_from_this() );
}

shared_ptr<Expr> WhileStmt::getExpr() {
	return expr;
}

shared_ptr<Stmt> WhileStmt::getBody() {
	return body;
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

void ForStmt::visit(shared_ptr<SyntaxVisitor> v) {
	v->accept( shared_from_this() );
}

shared_ptr<Stmt> ForStmt::getInit() {
	return init;
}

shared_ptr<Expr> ForStmt::getExpr() {
	return expr;
}

shared_ptr<Stmt> ForStmt::getInc() {
	return inc;
}

shared_ptr<Stmt> ForStmt::getBody() {
	return body;
}

bool ForStmt::hasInit() {
	return init != NULL;
}

bool ForStmt::hasExpr() {
	return expr != NULL;
}

bool ForStmt::hasInc() {
	return inc != NULL;
}

PrintStmt::PrintStmt(shared_ptr<Expr> e) {
	expr = e;
}

PrintStmt::~PrintStmt() {}

StmtStatus PrintStmt::execute( Stack &s, VarMap &m ) {

}

void PrintStmt::typeCheck( CheckState &cs ) {
	expr->typeCheck(cs);
}

void PrintStmt::visit(shared_ptr<SyntaxVisitor> v) {
	v->accept( shared_from_this() );
}

shared_ptr<Expr> PrintStmt::getExpr() {
	return expr;
}

EvalStmt::EvalStmt(shared_ptr<Expr> e) {
	expr = e;
}

EvalStmt::~EvalStmt() {}

StmtStatus EvalStmt::execute( Stack &s, VarMap &m ) {

}

void EvalStmt::typeCheck( CheckState &cs ) {
	expr->typeCheck(cs);
}

void EvalStmt::visit(shared_ptr<SyntaxVisitor> v) {
	v->accept( shared_from_this() );
}

shared_ptr<Expr> EvalStmt::getExpr() {
	return expr;
}

ReturnStmt::ReturnStmt() {
	expr = NULL;
}

ReturnStmt::ReturnStmt(shared_ptr<Expr> e) {
	expr = e;
}

ReturnStmt::~ReturnStmt() {}

StmtStatus ReturnStmt::execute( Stack &s, VarMap &m ) {


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

void ReturnStmt::visit(shared_ptr<SyntaxVisitor> v) {
	v->accept( shared_from_this() );
}

shared_ptr<Expr> ReturnStmt::getExpr() {
	return expr;
}

bool ReturnStmt::hasExpr() {
	return expr != NULL;
}

BreakStmt::BreakStmt() {}

BreakStmt::~BreakStmt() {}

StmtStatus BreakStmt::execute( Stack &s, VarMap &m ) {

}

void BreakStmt::typeCheck( CheckState & ) {
	// TODO unreachable code
}

void BreakStmt::visit(shared_ptr<SyntaxVisitor> v) {
	v->accept( shared_from_this() );
}

SwitchStmt::SwitchStmt( shared_ptr<Expr> e, map<shared_ptr<Expr>, shared_ptr<Stmt>> l, shared_ptr<Stmt> d ) {
	expr = e;
	list = l;
	def_stmt = d;
}

SwitchStmt::~SwitchStmt() {}

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

shared_ptr<Expr> SwitchStmt::getSwitch() {
	return expr;
}

map<shared_ptr<Expr>, shared_ptr<Stmt>> SwitchStmt::getCases() {
	return list;
}

shared_ptr<Stmt> SwitchStmt::getDefCase() {
	return def_stmt;
}

bool SwitchStmt::hasDefCase() {
	return def_stmt != NULL;
}


void SwitchStmt::visit(shared_ptr<SyntaxVisitor> v) {
	v->accept( shared_from_this() );
}


} /* namespace std */
