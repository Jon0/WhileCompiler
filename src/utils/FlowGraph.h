/*
 * FlowGraph.h
 *
 *  Created on: 19/04/2014
 *      Author: asdf
 */

#ifndef FLOWGRAPH_H_
#define FLOWGRAPH_H_

#include <memory>

#include "../lang/SyntaxVisitor.h"
#include "FlowNode.h"

namespace std {

class FlowGraph: public lang::SyntaxVisitor, public enable_shared_from_this<FlowGraph> {
public:
	FlowGraph();
	virtual ~FlowGraph();

	virtual void accept(shared_ptr<lang::Type>);
	virtual void accept(shared_ptr<lang::Value>);
	virtual void accept(shared_ptr<lang::Func>);
	virtual void accept(shared_ptr<lang::Program>);

	virtual void accept(shared_ptr<lang::BlockStmt>);
	virtual void accept(shared_ptr<lang::InitStmt>);
	virtual void accept(shared_ptr<lang::AssignStmt>);
	virtual void accept(shared_ptr<lang::IfStmt>);
	virtual void accept(shared_ptr<lang::WhileStmt>);
	virtual void accept(shared_ptr<lang::ForStmt>);
	virtual void accept(shared_ptr<lang::PrintStmt>);
	virtual void accept(shared_ptr<lang::EvalStmt>);
	virtual void accept(shared_ptr<lang::ReturnStmt>);
	virtual void accept(shared_ptr<lang::BreakStmt>);
	virtual void accept(shared_ptr<lang::SwitchStmt>);

	virtual void accept(shared_ptr<lang::ConstExpr>);
	virtual void accept(shared_ptr<lang::IsTypeExpr>);
	virtual void accept(shared_ptr<lang::VariableExpr>);
	virtual void accept(shared_ptr<lang::FuncCallExpr>);
	virtual void accept(shared_ptr<lang::RecordExpr>);

	virtual void accept(shared_ptr<lang::ListExpr>);
	virtual void accept(shared_ptr<lang::ListLengthExpr>);
	virtual void accept(shared_ptr<lang::ConcatExpr>);
	virtual void accept(shared_ptr<lang::ListLookupExpr>);
	virtual void accept(shared_ptr<lang::RecordMemberExpr>);
	virtual void accept(shared_ptr<lang::BasicCastExpr>);
	virtual void accept(shared_ptr<lang::AbstractOpExpr>);

private:
	shared_ptr<FlowNode> gr; // for each function

};

} /* namespace std */

#endif /* FLOWGRAPH_H_ */
