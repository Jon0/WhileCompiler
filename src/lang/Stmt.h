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

#include "Common.h"
#include "Expr.h"
#include "SyntaxElem.h"

namespace std {

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
	AssignStmt( Var, shared_ptr<Expr> );
	virtual ~AssignStmt();

	virtual StmtStatus execute( Stack &, VarMap & );
private:
	Var var;
	shared_ptr<Expr> expr;
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

class PrintStmt: public Stmt {
public:
	PrintStmt( shared_ptr<Expr> );
	virtual ~PrintStmt();

	virtual StmtStatus execute( Stack &, VarMap & );

private:
	shared_ptr<Expr> expr;
};

class ReturnStmt: public Stmt {
public:
	ReturnStmt( shared_ptr<Expr> );
	virtual ~ReturnStmt();

	virtual StmtStatus execute( Stack &, VarMap & );

private:
	shared_ptr<Expr> expr;
};

} /* namespace std */

#endif /* STMT_H_ */
