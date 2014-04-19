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

void Interpreter::accept(shared_ptr<Program> p) {
	p->visitFunc("main", shared_from_this());
}

void Interpreter::accept(shared_ptr<Func> f) {
	vars.push_back(VarMap());
	for (Var &v: f->getArgs()) {
		shared_ptr<Value> ev = stack.back()->clone(); // clone as type of original value
		stack.pop_back();
		vars.back().insert( VarMap::value_type(v, ev) );
	}

	isReturn = false;
	isBreak = false;
	f->getStmt()->visit( shared_from_this() );
	vars.pop_back();
}

void Interpreter::accept(shared_ptr<Type> t) {
}

void Interpreter::accept(shared_ptr<Value>) {}

void Interpreter::accept( shared_ptr<BlockStmt> bs ) {
	for (shared_ptr<Stmt> &s: bs->getStmt()) {
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

void Interpreter::accept(shared_ptr<InitStmt> is) {
	if (is->hasInit()) {

		// places result on eval stack
		is->getExpr()->visit( shared_from_this() );

		shared_ptr<Value> e = popValue();

		vars.back().insert(VarMap::value_type( is->getVar(), e->clone( is->getVar().type() ) ));	// always clone rhs on assignment
	}
	else {
		vars.back().insert(VarMap::value_type(is->getVar(), shared_ptr<Value>( new TypedValue<int>(is->getVar().type(), 0) )));
	}
}

void Interpreter::accept(shared_ptr<AssignStmt> as) {
	shared_ptr<Value> *assignable = NULL;

	// assignable uses a variable


	as->getLHS()->eval( stack, vars.back(), &assignable);
	if (!assignable) {
		throw runtime_error("interpreter error: lhs not assignable");
	}

	shared_ptr<Value> ev = evalClone( as->getRHS(), as->getRHS()->getType() ); // always clone rhs on assignment
	*assignable = ev;
}


void Interpreter::accept(shared_ptr<IfStmt> is) {
	shared_ptr<TypedValue<bool>> a = static_pointer_cast<TypedValue<bool>, Value>( eval( is->getExpr() ) );
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


void Interpreter::accept(shared_ptr<WhileStmt> ws) {
	shared_ptr<TypedValue<bool>> a = static_pointer_cast<TypedValue<bool>, Value>( eval( ws->getExpr() ) );
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
		a = static_pointer_cast<TypedValue<bool>, Value>( eval( ws->getExpr() ) );
	}
}

bool Interpreter::checkCond(shared_ptr<ForStmt> fs) {
	if (fs->hasExpr()) {
		shared_ptr<TypedValue<bool>> a = static_pointer_cast<TypedValue<bool>, Value>( eval( fs->getExpr() ) );
		return a->value();
	}
	else {
		return true; // when no condition eg. "for(;;){}"
	}
}

void Interpreter::accept(shared_ptr<ForStmt> fs) {
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

void Interpreter::accept(shared_ptr<PrintStmt> ps) {
	cout << eval( ps->getExpr() )->asString() << endl;
}

void Interpreter::accept(shared_ptr<EvalStmt> es) {
	eval( es->getExpr() );
}

void Interpreter::accept(shared_ptr<ReturnStmt> rs) {
	// put things on top of stack

	if (rs->hasExpr()) {
		shared_ptr<Value> ev = eval(rs->getExpr());
		stack.push_back( ev );
	}

	// escape current block
	isReturn = true;
}

void Interpreter::accept(shared_ptr<BreakStmt>) {
	// escape current block
	isBreak = true;
}

void Interpreter::accept(shared_ptr<SwitchStmt> ss) {


	shared_ptr<Value> ev = eval( ss->getSwitch() );

	for ( map<shared_ptr<Expr>, shared_ptr<Stmt>>::value_type ex: ss->getCases() ) {
		shared_ptr<Value> cv = eval(ex.first);

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

void Interpreter::accept(shared_ptr<ConstExpr>) {}

void Interpreter::accept(shared_ptr<IsTypeExpr>) {}

void Interpreter::accept(shared_ptr<VariableExpr>) {}

void Interpreter::accept(shared_ptr<FuncCallExpr>) {}
void Interpreter::accept(shared_ptr<RecordExpr>) {}

void Interpreter::accept(shared_ptr<ListExpr>) {}
void Interpreter::accept(shared_ptr<ListLengthExpr>) {}
void Interpreter::accept(shared_ptr<ConcatExpr>) {}
void Interpreter::accept(shared_ptr<ListLookupExpr>) {}
void Interpreter::accept(shared_ptr<RecordMemberExpr>) {}
void Interpreter::accept(shared_ptr<BasicCastExpr>) {}

void Interpreter::accept(shared_ptr<AbstractOpExpr>) {}

shared_ptr<Value> Interpreter::eval( shared_ptr<Expr> e ) {

}

shared_ptr<Value> Interpreter::evalClone( shared_ptr<Expr>, shared_ptr<Type> ) {

}

shared_ptr<Value> Interpreter::popValue() {
	if (evalStack.empty()) {
		throw runtime_error("value stack is empty");
	}

	shared_ptr<Value> ev = evalStack.back();
	evalStack.pop_back();
	return ev;
}


} /* namespace std */
