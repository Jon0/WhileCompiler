/*
 * Interpreter.cpp
 *
 *  Created on: 9/04/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "Interpreter.h"

#include "../lang/Program.h"
#include "../lang/Func.h"
#include "../lang/Stmt.h"
#include "../lang/Expr.h"

namespace std {

Interpreter::Interpreter() {
	isReturn = false;
	isBreak = false;
}

Interpreter::~Interpreter() {}

void Interpreter::accept(shared_ptr<lang::Program> p) {
	p->visitFunc("main", shared_from_this());
}

void Interpreter::accept(shared_ptr<lang::Func> f) {
	vars.push_back(lang::VarMap());
	for (auto &v: f->getArgs()) {
		auto ev = stack.back()->clone(); // clone as type of original value
		stack.pop_back();
		vars.back().insert( {v, ev} );
	}

	isReturn = false;
	isBreak = false;
	f->getStmt()->visit( shared_from_this() );
	vars.pop_back();
}

void Interpreter::accept(shared_ptr<lang::Type> t) {
}

void Interpreter::accept(shared_ptr<lang::Value>) {}

void Interpreter::accept( shared_ptr<lang::BlockStmt> bs ) {
	for (shared_ptr<lang::Stmt> &s: bs->getStmt()) {
			//StmtStatus ss = s->execute( stack, vars.back() );

			s->visit( shared_from_this() );
			if (isReturn) {
				isBreak = false;
				return;
			}
			else if (isBreak) {
				isReturn = false;
				isBreak = false;
				return;
			}
		}
}

void Interpreter::accept(shared_ptr<lang::InitStmt> is) {
	if (is->hasInit()) {

		// places result on eval stack
		is->getExpr()->visit( shared_from_this() );

		auto e = popValue();

		vars.back().insert( {is->getVar(), e->clone( is->getVar().type() )} );	// always clone rhs on assignment
	}
	else {
		vars.back().insert( {is->getVar(), shared_ptr<lang::Value>( new lang::TypedValue<int>(is->getVar().type(), 0) )} );
	}
}

void Interpreter::accept(shared_ptr<lang::AssignStmt> as) {
	shared_ptr<lang::Value> *assignable = NULL;

	// assignable uses a variable


	as->getLHS()->eval( stack, vars.back(), &assignable);
	if (!assignable) {
		throw runtime_error("interpreter error: lhs not assignable");
	}

	auto ev = evalClone( as->getRHS(), as->getRHS()->getType() ); // always clone rhs on assignment
	*assignable = ev;
}


void Interpreter::accept(shared_ptr<lang::IfStmt> is) {
	auto a = static_pointer_cast<lang::TypedValue<bool>, lang::Value>( eval( is->getExpr() ) );
	if ( a->value() ) {
		is->getBody()->visit(shared_from_this());
		if (isReturn) {
			return; 	// propogate back
		}
		else if (isBreak) {
			isBreak = false;
			return;
		}
	}

	// check alt exists
	else if ( is->hasAlt() ) {
		is->getAlt()->visit(shared_from_this());
		if (isReturn) {
			return; 	// propogate back
		}
		else if (isBreak) {
			isBreak = false;
			return;
		}
	}
}


void Interpreter::accept(shared_ptr<lang::WhileStmt> ws) {
	auto a = static_pointer_cast<lang::TypedValue<bool>, lang::Value>( eval( ws->getExpr() ) );
	while ( a->value() ) {
		ws->getBody()->visit(shared_from_this());
		if (isReturn) {
			return; 	// propogate back
		}
		else if (isBreak) {
			isBreak = false;
			return;
		}

		// reevaluate loop condition
		a = static_pointer_cast<lang::TypedValue<bool>, lang::Value>( eval( ws->getExpr() ) );
	}
}

bool Interpreter::checkCond(shared_ptr<lang::ForStmt> fs) {
	if (fs->hasExpr()) {
		auto a = static_pointer_cast<lang::TypedValue<bool>, lang::Value>( eval( fs->getExpr() ) );
		return a->value();
	}
	else {
		return true; // when no condition eg. "for(;;){}"
	}
}

void Interpreter::accept(shared_ptr<lang::ForStmt> fs) {
	// init
	if (fs->hasInit()) fs->getInit()->visit(shared_from_this());

	while ( checkCond(fs) ) {
		fs->getBody()->visit(shared_from_this());
		if (isReturn) {
			return; 	// propogate back
		}
		else if (isBreak) {
			isBreak = false;
			return;
		}

		// inc and reevaluate loop condition
		if (fs->hasInc()) fs->getInc()->visit(shared_from_this());
	}
}

void Interpreter::accept(shared_ptr<lang::PrintStmt> ps) {
	cout << eval( ps->getExpr() )->asString() << endl;
}

void Interpreter::accept(shared_ptr<lang::EvalStmt> es) {
	eval( es->getExpr() );
}

void Interpreter::accept(shared_ptr<lang::ReturnStmt> rs) {
	// put things on top of stack

	if (rs->hasExpr()) {
		shared_ptr<lang::Value> ev = eval(rs->getExpr());
		stack.push_back( ev );
	}

	// escape current block
	isReturn = true;
}

void Interpreter::accept(shared_ptr<lang::BreakStmt>) {
	// escape current block
	isBreak = true;
}

void Interpreter::accept(shared_ptr<lang::SwitchStmt> ss) {


	shared_ptr<lang::Value> ev = eval( ss->getSwitch() );

	for ( map<shared_ptr<lang::Expr>, shared_ptr<lang::Stmt>>::value_type ex: ss->getCases() ) {
		auto cv = eval(ex.first);

		if (*cv == *ev) {
			ex.second->visit(shared_from_this());
			isBreak = false;
			return;
		}
	}

	if (ss->hasDefCase()) {
		ss->getDefCase()->visit(shared_from_this());
		isBreak = false;
	}
}


/*
 * Expression evaluation, places results on eval stack
 */

void Interpreter::accept(shared_ptr<lang::ConstExpr>) {}

void Interpreter::accept(shared_ptr<lang::IsTypeExpr>) {}

void Interpreter::accept(shared_ptr<lang::VariableExpr>) {}

void Interpreter::accept(shared_ptr<lang::FuncCallExpr>) {}
void Interpreter::accept(shared_ptr<lang::RecordExpr>) {}

void Interpreter::accept(shared_ptr<lang::ListExpr>) {}
void Interpreter::accept(shared_ptr<lang::ListLengthExpr>) {}
void Interpreter::accept(shared_ptr<lang::ConcatExpr>) {}
void Interpreter::accept(shared_ptr<lang::ListLookupExpr>) {}
void Interpreter::accept(shared_ptr<lang::RecordMemberExpr>) {}
void Interpreter::accept(shared_ptr<lang::BasicCastExpr>) {}

void Interpreter::accept(shared_ptr<lang::AbstractOpExpr>) {}

shared_ptr<lang::Value> Interpreter::eval( shared_ptr<lang::Expr> e ) {
	// TODO
}

shared_ptr<lang::Value> Interpreter::evalClone( shared_ptr<lang::Expr>, shared_ptr<lang::Type> ) {
	// TODO
}

shared_ptr<lang::Value> Interpreter::popValue() {
	if (evalStack.empty()) {
		throw runtime_error("value stack is empty");
	}

	shared_ptr<lang::Value> ev = evalStack.back();
	evalStack.pop_back();
	return ev;
}


} /* namespace std */
