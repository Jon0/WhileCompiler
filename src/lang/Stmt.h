/*
 * Stmt.h
 *
 *  Created on: 16/03/2014
 *      Author: remnanjona
 */

#ifndef STMT_H_
#define STMT_H_

#include <map>
#include <string>

#include "SyntaxElem.h"
#include "Var.h"

namespace lang {
using namespace std;

class Expr;
class Type;

class Stmt: public SyntaxElem {
public:
	virtual ~Stmt() {}
	virtual void typeCheck( CheckState & ) = 0;
};

class BlockStmt: public Stmt, public enable_shared_from_this<BlockStmt> {
public:
	BlockStmt( vector<shared_ptr<Stmt>> );
	virtual ~BlockStmt();
	virtual void typeCheck( CheckState & );
	virtual void visit(shared_ptr<SyntaxVisitor>);
	vector<shared_ptr<Stmt>> getStmt();

private:
	vector<shared_ptr<Stmt>> block;
};

class InitStmt: public Stmt, public enable_shared_from_this<InitStmt> {
public:
	InitStmt( Var );
	InitStmt( Var, shared_ptr<Expr> );
	virtual ~InitStmt();

	virtual StmtStatus execute( Stack &, VarMap & );
	virtual void typeCheck( CheckState & );
	virtual void visit(shared_ptr<SyntaxVisitor>);

	bool hasInit();
	Var getVar();
	shared_ptr<Expr> getExpr();	// initialising expression
private:
	Var var;
	shared_ptr<Expr> expr;
};

class AssignStmt: public Stmt, public enable_shared_from_this<AssignStmt> {
public:
	AssignStmt( shared_ptr<Expr>, shared_ptr<Expr> );
	virtual ~AssignStmt();

	virtual StmtStatus execute( Stack &, VarMap & );
	virtual void typeCheck( CheckState & );
	virtual void visit(shared_ptr<SyntaxVisitor>);

	shared_ptr<Expr> getLHS();
	shared_ptr<Expr> getRHS();

private:
	shared_ptr<Expr> lhs;
	shared_ptr<Expr> rhs;
};

class IfStmt: public Stmt, public enable_shared_from_this<IfStmt> {
public:
	IfStmt( shared_ptr<Expr>, shared_ptr<Stmt>, shared_ptr<Stmt> );
	virtual ~IfStmt();

	virtual StmtStatus execute( Stack &, VarMap & );
	virtual void typeCheck( CheckState & );
	virtual void visit(shared_ptr<SyntaxVisitor>);

	shared_ptr<Expr> getExpr();
	shared_ptr<Stmt> getBody();
	shared_ptr<Stmt> getAlt();
	bool hasAlt();

private:
	shared_ptr<Expr> expr;
	shared_ptr<Stmt> body;
	shared_ptr<Stmt> alt;
};

class WhileStmt: public Stmt, public enable_shared_from_this<WhileStmt> {
public:
	WhileStmt( shared_ptr<Expr>, shared_ptr<Stmt> );
	virtual ~WhileStmt();

	virtual StmtStatus execute( Stack &, VarMap & );
	virtual void typeCheck( CheckState & );
	virtual void visit(shared_ptr<SyntaxVisitor>);

	shared_ptr<Expr> getExpr();
	shared_ptr<Stmt> getBody();

private:
	shared_ptr<Expr> expr;
	shared_ptr<Stmt> body;
};

class ForStmt: public Stmt, public enable_shared_from_this<ForStmt> {
public:
	ForStmt( shared_ptr<Stmt>, shared_ptr<Expr>, shared_ptr<Stmt>, shared_ptr<Stmt> );
	virtual ~ForStmt();

	virtual StmtStatus execute( Stack &, VarMap & );
	virtual void typeCheck( CheckState & );
	virtual void visit(shared_ptr<SyntaxVisitor>);

	shared_ptr<Stmt> getInit();
	shared_ptr<Expr> getExpr();
	shared_ptr<Stmt> getInc();
	shared_ptr<Stmt> getBody();
	bool hasInit();
	bool hasExpr();
	bool hasInc();

private:
	shared_ptr<Stmt> init;
	shared_ptr<Expr> expr;
	shared_ptr<Stmt> inc;
	shared_ptr<Stmt> body;
};

class PrintStmt: public Stmt, public enable_shared_from_this<PrintStmt> {
public:
	PrintStmt( shared_ptr<Expr> );
	virtual ~PrintStmt();

	virtual StmtStatus execute( Stack &, VarMap & );
	virtual void typeCheck( CheckState & );
	virtual void visit(shared_ptr<SyntaxVisitor>);

	shared_ptr<Expr> getExpr();

private:
	shared_ptr<Expr> expr;
};

class EvalStmt: public Stmt, public enable_shared_from_this<EvalStmt> {
public:
	EvalStmt( shared_ptr<Expr> );
	virtual ~EvalStmt();

	virtual StmtStatus execute( Stack &, VarMap & );
	virtual void typeCheck( CheckState & );
	virtual void visit(shared_ptr<SyntaxVisitor>);

	shared_ptr<Expr> getExpr();

private:
	shared_ptr<Expr> expr;
};

class ReturnStmt: public Stmt, public enable_shared_from_this<ReturnStmt> {
public:
	ReturnStmt();
	ReturnStmt( shared_ptr<Expr> );
	virtual ~ReturnStmt();

	virtual StmtStatus execute( Stack &, VarMap & );
	virtual void typeCheck( CheckState & );
	virtual void visit(shared_ptr<SyntaxVisitor>);

	shared_ptr<Expr> getExpr();
	bool hasExpr();

private:
	shared_ptr<Expr> expr;
};

class BreakStmt: public Stmt, public enable_shared_from_this<BreakStmt> {
public:
	BreakStmt();
	virtual ~BreakStmt();

	virtual StmtStatus execute( Stack &, VarMap & );
	virtual void typeCheck( CheckState & );
	virtual void visit(shared_ptr<SyntaxVisitor>);
};

class SwitchStmt: public Stmt, public enable_shared_from_this<SwitchStmt> {
public:
	SwitchStmt( shared_ptr<Expr>, map<shared_ptr<Expr>, shared_ptr<Stmt>>, shared_ptr<Stmt> );
	virtual ~SwitchStmt();

	virtual void typeCheck( CheckState &cs );

	shared_ptr<Expr> getSwitch() const;
	map<shared_ptr<Expr>, shared_ptr<Stmt>> getCases() const;
	shared_ptr<Stmt> getDefCase() const;
	bool hasDefCase() const;

	virtual void visit(shared_ptr<SyntaxVisitor>);

private:
	shared_ptr<Expr> expr;
	map<shared_ptr<Expr>, shared_ptr<Stmt>> list;
	shared_ptr<Stmt> def_stmt;

};

} /* namespace lang */

#endif /* STMT_H_ */
