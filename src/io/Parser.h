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

typedef pair<shared_ptr<Expr>, shared_ptr<Expr>> ExprPair;

template<class I, class O> class AddParser {
public:
	static O func( ExprPair in ) { // shared_ptr<Type> type, shared_ptr<Value> v
		return shared_ptr<Expr>( new OpExpr<I, I, AddOp<I>>(in.first, in.second) );
	}
};

template<class I, class O> class SubParser {
public:
	static O func( ExprPair in ) { // shared_ptr<Type> type, shared_ptr<Value> v
		return shared_ptr<Expr>( new OpExpr<I, I, SubOp<I>>(in.first, in.second) );
	}
};

template<class I, class O> class MulParser {
public:
	static O func( ExprPair in ) { // shared_ptr<Type> type, shared_ptr<Value> v
		return shared_ptr<Expr>( new OpExpr<I, I, MulOp<I>>(in.first, in.second) );
	}
};

template<class I, class O> class DivParser {
public:
	static O func( ExprPair in ) { // shared_ptr<Type> type, shared_ptr<Value> v
		return shared_ptr<Expr>( new OpExpr<I, I, DivOp<I>>(in.first, in.second) );
	}
};

template<class I, class O> class ModParser {
public:
	static O func( ExprPair in ) { // shared_ptr<Type> type, shared_ptr<Value> v
		return shared_ptr<Expr>( new OpExpr<I, I, ModOp<I>>(in.first, in.second) );
	}
};

template<class I, class O> class GreaterParser {
public:
	static O func(ExprPair in) { // shared_ptr<Type> type, shared_ptr<Value> v
		return shared_ptr<Expr>(
				new OpExpr<bool, I, GreaterOp<I>>(
						shared_ptr<Type>(new AtomicType<bool>("bool")),
						in.first, in.second));
	}
};

template<class I, class O> class GreaterEqualParser {
public:
	static O func(ExprPair in) { // shared_ptr<Type> type, shared_ptr<Value> v
		return shared_ptr<Expr>(
				new OpExpr<bool, I, GreaterEqualOp<I>>(
						shared_ptr<Type>(new AtomicType<bool>("bool")),
						in.first, in.second));
	}
};

template<class I, class O> class LessParser {
public:
	static O func(ExprPair in) { // shared_ptr<Type> type, shared_ptr<Value> v
		return shared_ptr<Expr>(
				new OpExpr<bool, I, LessOp<I>>(
						shared_ptr<Type>(new AtomicType<bool>("bool")),
						in.first, in.second));
	}
};

template<class I, class O> class LessEqualParser {
public:
	static O func(ExprPair in) { // shared_ptr<Type> type, shared_ptr<Value> v
		return shared_ptr<Expr>(
				new OpExpr<bool, I, LessEqualOp<I>>(
						shared_ptr<Type>(new AtomicType<bool>("bool")),
						in.first, in.second));
	}
};

class Parser {
public:
	Parser(Lexer &);
	virtual ~Parser();

	Program read();

private:
	ParserInput in;

	// list global functions and types
	FuncMap functions;
	map<string, shared_ptr<Type>> dectypes;
	map<string, shared_ptr<Expr>> constvals;

	Func readFunc();

	shared_ptr<Expr> readExpr(ParserContext &);
	shared_ptr<Expr> readExprPrimary(ParserContext &);
	shared_ptr<Expr> readExprExt(ParserContext &, shared_ptr<Expr> );
	shared_ptr<Expr> readAssignExpr(ParserContext &);

	shared_ptr<Stmt> readStmtBlock(ParserContext &);
	shared_ptr<Stmt> readStmt(ParserContext &);
	shared_ptr<Type> readType();
	shared_ptr<Type> readTypeInner();

	shared_ptr<Expr> readConstExpr();
	shared_ptr<Stmt> readVariableAssign(ParserContext &);

	/*
	 * const
	 */
	shared_ptr<Expr> intzero;

	shared_ptr<Type> stringtype;
	shared_ptr<Type> nulltype;
	shared_ptr<Type> booltype;

	shared_ptr<Value> nullvalue;
	shared_ptr<Value> truevalue;
	shared_ptr<Value> falsevalue;
};

} /* namespace std */

#endif /* PARSER_H_ */
