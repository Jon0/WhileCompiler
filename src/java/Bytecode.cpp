/*
 * Bytecode.cpp
 *
 *  Created on: 19/04/2014
 *      Author: asdf
 */

#include <iostream>

#include "../lang/Program.h"
#include "Bytecode.h"

namespace std {

Bytecode::Bytecode() {
	// TODO Auto-generated constructor stub

}

Bytecode::~Bytecode() {
	// TODO Auto-generated destructor stub
}

Classfile &Bytecode::getClassFile() {
	return cf;
}

void Bytecode::accept(shared_ptr<Type>) {}
void Bytecode::accept(shared_ptr<Value>) {}
void Bytecode::accept(shared_ptr<Func>) {}

/* create flow for each function */
void Bytecode::accept(shared_ptr<Program> p) {
	FuncMap fm = p->getFuncMap();

	for ( FuncMap::value_type func: fm ) {
		//func.first
	}
}

void Bytecode::accept(shared_ptr<BlockStmt>) {}
void Bytecode::accept(shared_ptr<InitStmt>) {}
void Bytecode::accept(shared_ptr<AssignStmt>) {}
void Bytecode::accept(shared_ptr<IfStmt>) {}
void Bytecode::accept(shared_ptr<WhileStmt>) {}
void Bytecode::accept(shared_ptr<ForStmt>) {}
void Bytecode::accept(shared_ptr<PrintStmt>) {}
void Bytecode::accept(shared_ptr<EvalStmt>) {}
void Bytecode::accept(shared_ptr<ReturnStmt>) {}
void Bytecode::accept(shared_ptr<BreakStmt>) {}
void Bytecode::accept(shared_ptr<SwitchStmt>) {}

void Bytecode::accept(shared_ptr<ConstExpr>) {}
void Bytecode::accept(shared_ptr<IsTypeExpr>) {}
void Bytecode::accept(shared_ptr<VariableExpr>) {}
void Bytecode::accept(shared_ptr<FuncCallExpr>) {}
void Bytecode::accept(shared_ptr<RecordExpr>) {}

void Bytecode::accept(shared_ptr<ListExpr>) {}
void Bytecode::accept(shared_ptr<ListLengthExpr>) {}
void Bytecode::accept(shared_ptr<ConcatExpr>) {}
void Bytecode::accept(shared_ptr<ListLookupExpr>) {}
void Bytecode::accept(shared_ptr<RecordMemberExpr>) {}
void Bytecode::accept(shared_ptr<BasicCastExpr>) {}
void Bytecode::accept(shared_ptr<AbstractOpExpr>) {}

} /* namespace std */
