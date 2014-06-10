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

class WhileToX86: public SyntaxVisitor, public enable_shared_from_this<WhileToX86> {
public:
	WhileToX86(shared_ptr<X86Program>, bool);
	virtual ~WhileToX86();

	virtual void accept(shared_ptr<Type>);
	virtual void accept(shared_ptr<Value>);
	virtual void accept(shared_ptr<Func>);
	virtual void accept(shared_ptr<Program>);

	virtual void accept(shared_ptr<BlockStmt>);
	virtual void accept(shared_ptr<InitStmt>);
	virtual void accept(shared_ptr<AssignStmt>);
	virtual void accept(shared_ptr<IfStmt>);
	virtual void accept(shared_ptr<WhileStmt>);
	virtual void accept(shared_ptr<ForStmt>);
	virtual void accept(shared_ptr<PrintStmt>);
	virtual void accept(shared_ptr<EvalStmt>);
	virtual void accept(shared_ptr<ReturnStmt>);
	virtual void accept(shared_ptr<BreakStmt>);
	virtual void accept(shared_ptr<SwitchStmt>);

	virtual void accept(shared_ptr<ConstExpr>);
	virtual void accept(shared_ptr<IsTypeExpr>);
	virtual void accept(shared_ptr<VariableExpr>);
	virtual void accept(shared_ptr<FuncCallExpr>);
	virtual void accept(shared_ptr<RecordExpr>);

	virtual void accept(shared_ptr<ListExpr>);
	virtual void accept(shared_ptr<ListLengthExpr>);
	virtual void accept(shared_ptr<ConcatExpr>);
	virtual void accept(shared_ptr<ListLookupExpr>);
	virtual void accept(shared_ptr<RecordMemberExpr>);
	virtual void accept(shared_ptr<BasicCastExpr>);
	virtual void accept(shared_ptr<AbstractOpExpr>);
	virtual void accept(shared_ptr<EquivOp>);
	virtual void accept(shared_ptr<NotEquivOp>);
	virtual void accept(shared_ptr<AndExpr>);
	virtual void accept(shared_ptr<OrExpr>);
	virtual void accept(shared_ptr<NotExpr>);

private:
	shared_ptr<X86Program> out;

	int dLabel, tagCount;


	// used to track values
	bool clone_objs;	// variable expressions use cloning
	string loopbreak; 	// tag to leave current loop
	objmap refs;
	objstack top;
	shared_ptr<WhileObject> returnSpace;

	// define the external library functions
	// these get linked with gcc
	shared_ptr<X86Function> print, equiv, clone, append;

	// add debug annotations
	bool debug_out;

	shared_ptr<WhileObject> popRef();
	shared_ptr<WhileObject> popRefAndCopy();
	shared_ptr<WhileObject> objFromExpr( shared_ptr<Expr>, bool );

	// no l0nger using these
	shared_ptr<X86Register> refIntoReg(shared_ptr<X86Reference>);
	int getTypeSize(shared_ptr<Type>);
	int getTypeTag(shared_ptr<Type>);
};

} /* namespace std */

#endif /* WHILETOX86_H_ */
