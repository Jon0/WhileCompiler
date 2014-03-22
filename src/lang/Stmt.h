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

namespace std {

class Expr;
class Type;

class Stmt: public SyntaxElem {
public:
	virtual ~Stmt() {}
	virtual StmtStatus execute( Stack &, VarMap & ) = 0;
};

class BlockStmt: public Stmt {
public:
	BlockStmt( vector<shared_ptr<Stmt>> );
	virtual ~BlockStmt();
	virtual StmtStatus execute( Stack &, VarMap & );
private:
	vector<shared_ptr<Stmt>> block;
};

class InitStmt: public Stmt {
public:
	InitStmt( Var );
	InitStmt( Var, shared_ptr<Expr> );
	virtual ~InitStmt();

	virtual StmtStatus execute( Stack &, VarMap & );
private:
	Var var;
	shared_ptr<Expr> expr;
};

class AssignStmt: public Stmt {
public:
	AssignStmt( shared_ptr<Expr>, shared_ptr<Expr> );
	virtual ~AssignStmt();

	virtual StmtStatus execute( Stack &, VarMap & );
private:
	shared_ptr<Expr> lhs;
	shared_ptr<Expr> rhs;
};

class ListAssignStmt: public Stmt {
public:
	ListAssignStmt( Var, shared_ptr<Expr>, shared_ptr<Expr> );
	virtual ~ListAssignStmt();

	virtual StmtStatus execute( Stack &, VarMap & );
private:
	Var var;
	shared_ptr<Expr> expr;
	shared_ptr<Expr> ind;
};

class RecordAssignStmt: public Stmt {
public:
	RecordAssignStmt( Var, shared_ptr<Expr>, string );
	virtual ~RecordAssignStmt();

	virtual StmtStatus execute( Stack &, VarMap & );
private:
	Var var;
	shared_ptr<Expr> expr;
	string member;
};

class IfStmt: public Stmt {
public:
	IfStmt( shared_ptr<Expr>, shared_ptr<Stmt>, shared_ptr<Stmt> );
	virtual ~IfStmt();

	virtual StmtStatus execute( Stack &, VarMap & );

private:
	shared_ptr<Expr> expr;
	shared_ptr<Stmt> body;
	shared_ptr<Stmt> alt;
};

class WhileStmt: public Stmt {
public:
	WhileStmt( shared_ptr<Expr>, shared_ptr<Stmt> );
	virtual ~WhileStmt();

	virtual StmtStatus execute( Stack &, VarMap & );

private:
	shared_ptr<Expr> expr;
	shared_ptr<Stmt> body;
};

class ForStmt: public Stmt {
public:
	ForStmt( shared_ptr<Stmt>, shared_ptr<Expr>, shared_ptr<Stmt>, shared_ptr<Stmt> );
	virtual ~ForStmt();

	virtual StmtStatus execute( Stack &, VarMap & );

private:
	bool checkCond( Stack &, VarMap & );
	shared_ptr<Stmt> init;
	shared_ptr<Expr> expr;
	shared_ptr<Stmt> inc;
	shared_ptr<Stmt> body;
};

class PrintStmt: public Stmt {
public:
	PrintStmt( shared_ptr<Expr> );
	virtual ~PrintStmt();

	virtual StmtStatus execute( Stack &, VarMap & );

private:
	shared_ptr<Expr> expr;
};

class EvalStmt: public Stmt {
public:
	EvalStmt( shared_ptr<Expr> );
	virtual ~EvalStmt();

	virtual StmtStatus execute( Stack &, VarMap & );

private:
	shared_ptr<Expr> expr;
};

class ReturnStmt: public Stmt {
public:
	ReturnStmt();
	ReturnStmt( shared_ptr<Expr> );
	virtual ~ReturnStmt();

	virtual StmtStatus execute( Stack &, VarMap & );

private:
	shared_ptr<Expr> expr;
};

class BreakStmt: public Stmt {
public:
	BreakStmt();
	virtual ~BreakStmt();

	virtual StmtStatus execute( Stack &, VarMap & );

private:
	shared_ptr<Expr> expr;
};

class SwitchStmt: public Stmt {
public:
	SwitchStmt( shared_ptr<Expr>, map<shared_ptr<Expr>, shared_ptr<Stmt>>, shared_ptr<Stmt> );
	virtual ~SwitchStmt();

	virtual StmtStatus execute( Stack &, VarMap & );

private:
	shared_ptr<Expr> expr;
	map<shared_ptr<Expr>, shared_ptr<Stmt>> list;
	shared_ptr<Stmt> def_stmt;

};

} /* namespace std */

#endif /* STMT_H_ */
