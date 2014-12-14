#ifndef PARSER_H_
#define PARSER_H_

#include <memory>

#include "../lang/Expr.h"
#include "../lang/Func.h"
#include "../lang/Program.h"
#include "../lang/Stmt.h"
#include "../lang/Type.h"
#include "OpParser.h"
#include "ParserAttr.h"

namespace io {
namespace parser {

class WhileParser {
public:
	WhileParser(Lexer &);
	virtual ~WhileParser();

	std::shared_ptr<lang::Program> read();

private:
	string classname;
	ParserInput in;

	// list global functions and types
	lang::FuncMap functions;
	std::map<std::string, std::shared_ptr<lang::Type>> dectypes;
	std::map<std::string, std::shared_ptr<lang::Expr>> constvals;

	std::vector<std::shared_ptr<lang::Var>> unresolved;

	std::shared_ptr<lang::Func> readFunc();

	std::shared_ptr<lang::Expr> readExpr(ParserContext &);

	/**
	 * comparison expressions
	 */
	std::shared_ptr<lang::Expr> readExprCmpr(ParserContext &);

	/**
	 * list expressions
	 */
	std::shared_ptr<lang::Expr> readExprList(ParserContext &);

	/** 
	 * math expresions
	 */
	std::shared_ptr<lang::Expr> readExprAdd(ParserContext &);
	std::shared_ptr<lang::Expr> readExprMul(ParserContext &);

	/**
	 * list expressions
	 */
	std::shared_ptr<lang::Expr> readExprTerm(ParserContext &);
	std::shared_ptr<lang::Expr> readExprTermInner(ParserContext &);

	std::shared_ptr<lang::Expr> readExprPrimary(ParserContext &);

	// char and numerical parsing
	std::shared_ptr<lang::Expr> readConstExpr();

	// creates unresolved var type
	std::shared_ptr<lang::Var> unresolvedVar(Token);

	std::shared_ptr<lang::Stmt> readStmtBlock(ParserContext &);
	std::shared_ptr<lang::Stmt> readStmt(ParserContext &);
	std::shared_ptr<lang::Type> readType();
	std::shared_ptr<lang::Type> readTypeInner();


	std::shared_ptr<lang::Stmt> readVariableAssign(ParserContext &);

	/*
	 * TODO make static
	 */
	std::shared_ptr<lang::Expr> intzero;
	std::shared_ptr<lang::Expr> realzero;
	std::shared_ptr<lang::Expr> unknownzero;

	std::shared_ptr<lang::Type> stringtype;
	std::shared_ptr<lang::Type> nulltype;
	std::shared_ptr<lang::Type> booltype;

	std::shared_ptr<lang::Value> nullvalue;
	std::shared_ptr<lang::Value> truevalue;
	std::shared_ptr<lang::Value> falsevalue;
};

} /* namespace parser */
} /* namespace io */

#endif /* PARSER_H_ */
