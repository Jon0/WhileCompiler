/*
 * WhileToX86.h
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#ifndef WHILETOX86_H_
#define WHILETOX86_H_

#include <map>
#include <memory>

#include "../lang/SyntaxVisitor.h"
#include "X86Reference.h"
#include "X86StackFrame.h"

namespace std {

class Expr;
class WhileObject;

typedef map<string, shared_ptr<WhileObject>> objmap;
typedef vector<shared_ptr<WhileObject>> objstack;

class X86Function;
class X86Program;

class WhileToX86: public lang::SyntaxVisitor, public enable_shared_from_this<WhileToX86> {
public:
	WhileToX86(shared_ptr<X86Program>, bool);
	virtual ~WhileToX86();

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
	virtual void accept(shared_ptr<lang::EquivOp>);
	virtual void accept(shared_ptr<lang::NotEquivOp>);
	virtual void accept(shared_ptr<lang::AndExpr>);
	virtual void accept(shared_ptr<lang::OrExpr>);
	virtual void accept(shared_ptr<lang::NotExpr>);

private:
	shared_ptr<X86Program> out;

	int dLabel, tagCount;


	// used to track values
	bool clone_objs;	// variable expressions use cloning
	string loopbreak; 	// tag to leave current loop
	objmap refs;
	objstack top;
	shared_ptr<WhileObject> returnSpace;

	// add debug annotations
	bool debug_out;

	shared_ptr<WhileObject> popRef();
	shared_ptr<WhileObject> popRefAndCopy();
	shared_ptr<WhileObject> objFromExpr( shared_ptr<lang::Expr>, bool );
	shared_ptr<WhileObject> objFromExpr( shared_ptr<lang::Expr> ); // does not modify cloning mode

	// no l0nger using these
	shared_ptr<X86Register> refIntoReg(shared_ptr<X86Reference>);
	int getTypeSize(shared_ptr<lang::Type>);
	int getTypeTag(shared_ptr<lang::Type>);
};

} /* namespace std */

#endif /* WHILETOX86_H_ */
