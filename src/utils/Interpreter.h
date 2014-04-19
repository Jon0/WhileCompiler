#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <memory>
#include <vector>

#include "../lang/SyntaxVisitor.h"
#include "../lang/Var.h"

namespace std {

class Expr;

typedef vector<shared_ptr<Value>> Stack;

class Interpreter: public SyntaxVisitor, public enable_shared_from_this<Interpreter> {
public:
	Interpreter();
	virtual ~Interpreter();

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
	bool checkCond( shared_ptr<ForStmt> );
	shared_ptr<Value> eval( shared_ptr<Expr> );
	shared_ptr<Value> evalClone( shared_ptr<Expr>, shared_ptr<Type> );

	shared_ptr<Value> popValue();

	Stack stack;
	vector<VarMap> vars;
	vector<shared_ptr<Value>> evalStack;

	bool isReturn;
	bool isBreak;
};

} /* namespace std */

#endif /* INTERPRETER_H_ */
