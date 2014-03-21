#ifndef PARSER_H_
#define PARSER_H_

#include <memory>

#include "../lang/Expr.h"
#include "../lang/Func.h"
#include "../lang/Program.h"
#include "../lang/Stmt.h"
#include "../lang/Type.h"
#include "ParserAttr.h"

namespace std {

class Parser {
public:
	Parser(Lexer &);
	virtual ~Parser();

	Program read();

private:
	ParserInput in;

	// TODO list global functions and types
	FuncMap functions;
	map<string, shared_ptr<Type>> dectypes;
	map<string, shared_ptr<Expr>> constvals;

	Func readFunc();

	shared_ptr<Expr> readExpr(ParserContext &);
	shared_ptr<Expr> readAssignExpr(ParserContext &);

	shared_ptr<Stmt> readStmtBlock(ParserContext &);
	shared_ptr<Stmt> readStmt(ParserContext &);
	shared_ptr<Type> readType();

	shared_ptr<Expr> readConstExpr();
	shared_ptr<Stmt> readVariableAssign(ParserContext &);
};

} /* namespace std */

#endif /* PARSER_H_ */
