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
	virtual void execute( Stack &, VarMap & ) = 0;
};

class BlockStmt: public Stmt {
public:
	BlockStmt( vector<shared_ptr<Stmt>> );
	virtual ~BlockStmt();
	virtual void execute( Stack &, VarMap & );
private:
	vector<shared_ptr<Stmt>> block;
};

class InitStmt: public Stmt {
public:
	InitStmt( Var );
	InitStmt( Var, shared_ptr<Expr> );
	virtual ~InitStmt();

	virtual void execute( Stack &, VarMap & );
private:
	Var var;
	shared_ptr<Expr> expr;
};

class AssignStmt: public Stmt {
public:
	AssignStmt( Var, shared_ptr<Expr> );
	virtual ~AssignStmt();

	virtual void execute( Stack &, VarMap & );
private:
	Var var;
	shared_ptr<Expr> expr;
};

class ListAssignStmt: public Stmt {
public:
	ListAssignStmt( Var, shared_ptr<Expr>, shared_ptr<Expr> );
	virtual ~ListAssignStmt();

	virtual void execute( Stack &, VarMap & );
private:
	Var var;
	shared_ptr<Expr> expr;
	shared_ptr<Expr> ind;
};

class IfStmt: public Stmt {
public:
	IfStmt( shared_ptr<Expr>, shared_ptr<Stmt>, shared_ptr<Stmt> );
	virtual ~IfStmt();

	virtual void execute( Stack &, VarMap & );

private:
	shared_ptr<Expr> expr;
	shared_ptr<Stmt> body;
	shared_ptr<Stmt> alt;
};

class PrintStmt: public Stmt {
public:
	PrintStmt( shared_ptr<Expr> );
	virtual ~PrintStmt();

	virtual void execute( Stack &, VarMap & );

private:
	shared_ptr<Expr> expr;
};

class ReturnStmt: public Stmt {
public:
	ReturnStmt( shared_ptr<Expr> );
	virtual ~ReturnStmt();

	virtual void execute( Stack &, VarMap & );

private:
	shared_ptr<Expr> expr;
};

} /* namespace std */

#endif /* STMT_H_ */
