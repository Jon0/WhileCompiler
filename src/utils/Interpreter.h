#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <memory>
#include <vector>

#include "../lang/SyntaxVisitor.h"
#include "../lang/Var.h"

namespace lang {
	class Expr;
}

namespace std {

typedef vector<shared_ptr<lang::Value>> Stack;

class Interpreter: public lang::SyntaxVisitor, public enable_shared_from_this<Interpreter> {
public:
	Interpreter();
	virtual ~Interpreter();

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
	bool checkCond( shared_ptr<lang::ForStmt> );
	shared_ptr<lang::Value> eval( shared_ptr<lang::Expr> );
	shared_ptr<lang::Value> evalClone( shared_ptr<lang::Expr>, shared_ptr<lang::Type> );

	shared_ptr<lang::Value> popValue();

	Stack stack;
	vector<lang::VarMap> vars;
	vector<shared_ptr<lang::Value>> evalStack;

	// return and break states in execution
	bool isReturn;
	bool isBreak;
};

} /* namespace std */

#endif /* INTERPRETER_H_ */
