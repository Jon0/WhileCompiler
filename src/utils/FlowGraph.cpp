/*
 * FlowGraph.cpp
 *
 *  Created on: 19/04/2014
 *      Author: asdf
 */

#include "FlowGraph.h"

namespace std {

FlowGraph::FlowGraph() {
	// TODO Auto-generated constructor stub

}

FlowGraph::~FlowGraph() {
	// TODO Auto-generated destructor stub
}

void FlowGraph::accept(shared_ptr<Type>) {}
void FlowGraph::accept(shared_ptr<Value>) {}
void FlowGraph::accept(shared_ptr<Func>) {}
void FlowGraph::accept(shared_ptr<Program>) {}

void FlowGraph::accept(shared_ptr<BlockStmt>) {}
void FlowGraph::accept(shared_ptr<InitStmt>) {}
void FlowGraph::accept(shared_ptr<AssignStmt>) {}
void FlowGraph::accept(shared_ptr<IfStmt>) {}
void FlowGraph::accept(shared_ptr<WhileStmt>) {}
void FlowGraph::accept(shared_ptr<ForStmt>) {}
void FlowGraph::accept(shared_ptr<PrintStmt>) {}
void FlowGraph::accept(shared_ptr<EvalStmt>) {}
void FlowGraph::accept(shared_ptr<ReturnStmt>) {}
void FlowGraph::accept(shared_ptr<BreakStmt>) {}
void FlowGraph::accept(shared_ptr<SwitchStmt>) {}

void FlowGraph::accept(shared_ptr<ConstExpr>) {}
void FlowGraph::accept(shared_ptr<IsTypeExpr>) {}
void FlowGraph::accept(shared_ptr<VariableExpr>) {}
void FlowGraph::accept(shared_ptr<FuncCallExpr>) {}
void FlowGraph::accept(shared_ptr<RecordExpr>) {}

void FlowGraph::accept(shared_ptr<ListExpr>) {}
void FlowGraph::accept(shared_ptr<ListLengthExpr>) {}
void FlowGraph::accept(shared_ptr<ConcatExpr>) {}
void FlowGraph::accept(shared_ptr<ListLookupExpr>) {}
void FlowGraph::accept(shared_ptr<RecordMemberExpr>) {}
void FlowGraph::accept(shared_ptr<BasicCastExpr>) {}
void FlowGraph::accept(shared_ptr<AbstractOpExpr>) {}

} /* namespace std */

