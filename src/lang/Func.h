/*
 * Func.h
 *
 *  Created on: 16/03/2014
 *      Author: remnanjona
 */

#ifndef FUNC_H_
#define FUNC_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "SyntaxElem.h"
#include "Var.h"

namespace lang {
using namespace std;

class Expr;
class Stmt;
class Type;

class Func: public SyntaxElem, public enable_shared_from_this<Func> {
public:
	Func( string, shared_ptr<Type>, vector<Var>, shared_ptr<Stmt> );
	virtual ~Func();

	string name() const;
	int numArgs() const;
	shared_ptr<Type>  argType(int) const;
	shared_ptr<Type> returnType() const;
	vector<Var> getArgs() const;
	shared_ptr<Stmt> getStmt() const;

	void execute( Stack & );
	void typeCheck();

	virtual void visit(shared_ptr<SyntaxVisitor>);

private:
	// Type return_type;
	shared_ptr<Type> r_type;
	vector<Var> args;
	string name_str;
	shared_ptr<Stmt> stmts;
};

} /* namespace std */

#endif /* FUNC_H_ */
