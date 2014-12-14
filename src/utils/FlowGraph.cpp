/*
 * FlowGraph.cpp
 *
 *  Created on: 19/04/2014
 *      Author: asdf
 */

#include "../lang/Program.h"
#include "FlowGraph.h"

namespace std {

FlowGraph::FlowGraph() {
	// TODO Auto-generated constructor stub

}

FlowGraph::~FlowGraph() {
	// TODO Auto-generated destructor stub
}

void FlowGraph::accept(shared_ptr<lang::Type>) {}
void FlowGraph::accept(shared_ptr<lang::Value>) {}
void FlowGraph::accept(shared_ptr<lang::Func>) {}

/* create flow for each function */
void FlowGraph::accept(shared_ptr<lang::Program> p) {
	auto fm = p->getFuncMap();

	for ( lang::FuncMap::value_type &func: fm ) {
		//func.first
	}
}

void FlowGraph::accept(shared_ptr<lang::BlockStmt>) {}
void FlowGraph::accept(shared_ptr<lang::InitStmt>) {}
void FlowGraph::accept(shared_ptr<lang::AssignStmt>) {}
void FlowGraph::accept(shared_ptr<lang::IfStmt>) {}
void FlowGraph::accept(shared_ptr<lang::WhileStmt>) {}
void FlowGraph::accept(shared_ptr<lang::ForStmt>) {}
void FlowGraph::accept(shared_ptr<lang::PrintStmt>) {}
void FlowGraph::accept(shared_ptr<lang::EvalStmt>) {}
void FlowGraph::accept(shared_ptr<lang::ReturnStmt>) {}
void FlowGraph::accept(shared_ptr<lang::BreakStmt>) {}
void FlowGraph::accept(shared_ptr<lang::SwitchStmt>) {}

void FlowGraph::accept(shared_ptr<lang::ConstExpr>) {}
void FlowGraph::accept(shared_ptr<lang::IsTypeExpr>) {}
void FlowGraph::accept(shared_ptr<lang::VariableExpr>) {}
void FlowGraph::accept(shared_ptr<lang::FuncCallExpr>) {}
void FlowGraph::accept(shared_ptr<lang::RecordExpr>) {}

void FlowGraph::accept(shared_ptr<lang::ListExpr>) {}
void FlowGraph::accept(shared_ptr<lang::ListLengthExpr>) {}
void FlowGraph::accept(shared_ptr<lang::ConcatExpr>) {}
void FlowGraph::accept(shared_ptr<lang::ListLookupExpr>) {}
void FlowGraph::accept(shared_ptr<lang::RecordMemberExpr>) {}
void FlowGraph::accept(shared_ptr<lang::BasicCastExpr>) {}
void FlowGraph::accept(shared_ptr<lang::AbstractOpExpr>) {}

} /* namespace std */

