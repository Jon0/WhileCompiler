/*
 * Bytecode.h
 *
 *  Created on: 19/04/2014
 *      Author: asdf
 */

#ifndef BYTECODE_H_
#define BYTECODE_H_

#include <memory>

#include "../lang/SyntaxVisitor.h"
#include "Classfile.h"

namespace std {

class Bytecode: public SyntaxVisitor, public enable_shared_from_this<Bytecode> {
public:
	Bytecode();
	virtual ~Bytecode();

	Classfile &getClassFile();

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

private:
	Classfile cf;

};

} /* namespace std */

#endif /* BYTECODE_H_ */
