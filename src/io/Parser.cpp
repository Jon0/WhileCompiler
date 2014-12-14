#include <iostream>
#include <stdexcept>

#include "Parser.h"

namespace io {
namespace parser {

WhileParser::WhileParser(Lexer &lexer)
	:
	in(lexer) {

	// remove file extension to name class
	size_t ind = lexer.getFilename().find(".");
	classname = lexer.getFilename().substr(0, ind);

	// initialise atomic types
	std::vector<shared_ptr<lang::Type>> initial_types {
		lang::voidType,
		lang::nullType,
		lang::boolType,
		lang::charType,
		lang::intType,
		lang::realType
	};

	for (std::shared_ptr<lang::Type> p: initial_types) {
		dectypes.insert( {p->nameStr(), p} );
	}

	/*
	 * define string as list of char
	 */
	stringtype = std::make_shared<lang::ListType>( (*dectypes.find("char")).second, true );
	dectypes.insert( {"string", stringtype} );

	/*
	 * some internal constants
	 */
	std::shared_ptr<lang::Value> v1 = std::make_shared<lang::TypedValue<int>>((*dectypes.find("int")).second, 0);
	intzero = std::make_shared<lang::ConstExpr>(v1);

	std::shared_ptr<lang::Value> v2 = std::make_shared<lang::TypedValue<int>>((*dectypes.find("real")).second, 0);
	realzero = std::make_shared<lang::ConstExpr>(v2);

	nulltype = (*dectypes.find("null")).second;
	booltype = (*dectypes.find("bool")).second;

	nullvalue = std::make_shared<lang::NullValue>( nulltype );
	truevalue = std::make_shared<lang::TypedValue<bool>>( booltype, true );
	falsevalue = std::make_shared<lang::TypedValue<bool>>( booltype, false );


	/*
	 * define null, true and false as constants
	 */
	constvals.insert( {"null", std::make_shared<lang::ConstExpr>(nullvalue)} );
	constvals.insert( {"true", std::make_shared<lang::ConstExpr>(truevalue)} );
	constvals.insert( {"false", std::make_shared<lang::ConstExpr>(falsevalue)} );
}

WhileParser::~WhileParser() {}

std::shared_ptr<lang::Program> WhileParser::read() {
	try {
		while ( !in.empty() ) {
			if (in.canMatch("const")) {
				auto name = in.pop();
				in.match("is");
				ParserContext temp_ctxt;
				auto assoc = readExpr(temp_ctxt);
				constvals.insert( {name.text(), assoc} );
			}
			else if (in.canMatch("type")) {
				auto name = in.pop();
				in.match("is");
				auto type = readType();
				dectypes.insert( {name.text(), type->makeAlias( name.text() )} );
			}
			else {
				auto f = readFunc();
				functions.insert( {f->name(), f} );
			}
		}
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		throw std::runtime_error("failed to parse program");
	}

	return std::make_shared<lang::Program>(functions, classname);
}

std::shared_ptr<lang::Expr> WhileParser::readExpr(ParserContext &ctxt) {
	auto e = readExprCmpr(ctxt);
	Token tok = in.peek();

	/*
	 * Boolean operations
	 */
	bool read = true;
	while (read) {
		if (in.canMatch("&")) {
			in.match("&"); // second &
			auto second = readExpr(ctxt);
			e = std::make_shared<lang::AndExpr>(tok, booltype, e, second);
		} else if (in.canMatch("|")) {
			in.match("|"); // second |
			auto second = readExpr(ctxt);
			e = std::make_shared<lang::OrExpr>(tok, booltype, e, second);
		} else {
			read = false;
		}
	}
	return e;
}

std::shared_ptr<lang::Expr> WhileParser::readExprCmpr(ParserContext &ctxt) {
	auto e = readExprList(ctxt);
	auto tok = in.peek();

	/*
	 * Equality on any values
	 */
	if (in.canMatch("=")) {
		in.match("="); // second =
		auto second = readExprList(ctxt);
		e = std::make_shared<lang::EquivOp>(tok, booltype, e, second);
	}
	else if (in.canMatch("!")) {
		in.match("="); // second =
		auto second = readExprList(ctxt);
		e = std::make_shared<lang::NotEquivOp>(tok, booltype, e, second);
	}
	else if (in.canMatch(">")) {
		if (in.canMatch("=")) {
			ExprPair ep = {e, readExprList(ctxt)};
			e = lang::TypeSwitch<GreaterEqualParser, std::shared_ptr<lang::Expr>, ExprPair>::typeSwitch( e->getType(), ep );
		} else {
			ExprPair ep = {e, readExprList(ctxt)};
			e = lang::TypeSwitch<GreaterParser, std::shared_ptr<lang::Expr>, ExprPair>::typeSwitch( e->getType(), ep );
		}
	} else if (in.canMatch("<")) {
		if (in.canMatch("=")) {
			ExprPair ep = {e, readExprList(ctxt)};
			e = lang::TypeSwitch<LessEqualParser, std::shared_ptr<lang::Expr>, ExprPair>::typeSwitch( e->getType(), ep );
		} else {
			ExprPair ep = {e, readExprList(ctxt)};
			e = lang::TypeSwitch<LessParser, std::shared_ptr<lang::Expr>, ExprPair>::typeSwitch( e->getType(), ep );
		}
	}
	else if (in.canMatch("is")) {
		// type comparison
		auto ty = readType();
		e = std::make_shared<lang::IsTypeExpr>(tok, e, ty);
	}
	return e;
}

std::shared_ptr<lang::Expr> WhileParser::readExprList(ParserContext &ctxt) {
	auto e = readExprAdd(ctxt);
	auto tok = in.peek();

	// first expr is list (++ concat)
	if (in.canMatch("++")) {
		auto second = readExprList(ctxt);
		e = std::make_shared<lang::ConcatExpr>(tok, e->getType(), e, second); // TODO infer type from first term?
	}
	return e;
}

std::shared_ptr<lang::Expr> WhileParser::readExprAdd(ParserContext &ctxt) {
	auto e = readExprMul(ctxt);

	bool read = true;
	while (read) {
		if (in.canMatch("+")) {
			ExprPair ep = ExprPair(e, readExprMul(ctxt));
			e = lang::TypeSwitch<AddParser, std::shared_ptr<lang::Expr>, ExprPair>::typeSwitch(
					e->getType(), ep);
		} else if (in.canMatch("-")) {
			ExprPair ep = ExprPair(e, readExprMul(ctxt));
			e = lang::TypeSwitch<SubParser, std::shared_ptr<lang::Expr>, ExprPair>::typeSwitch(
					e->getType(), ep);
		} else {
			read = false;
		}
	}

	return e;
}

std::shared_ptr<lang::Expr> WhileParser::readExprMul(ParserContext &ctxt) {
	auto e = readExprTerm(ctxt);

	/*
	 * Math Operations
	 */
	bool read = true;
	while (read) {
		if (in.canMatch("*")) {
			ExprPair ep = ExprPair(e, readExprTerm(ctxt));
			e = lang::TypeSwitch<MulParser, std::shared_ptr<lang::Expr>, ExprPair>::typeSwitch(
					e->getType(), ep);
		} else if (in.canMatch("/")) {
			ExprPair ep = ExprPair(e, readExprTerm(ctxt));
			e = lang::TypeSwitch<DivParser, std::shared_ptr<lang::Expr>, ExprPair>::typeSwitch(
					e->getType(), ep);
		} else if (in.canMatch("%")) {
			ExprPair ep = ExprPair(e, readExprTerm(ctxt));
			e = lang::TypeSwitch<ModParser, std::shared_ptr<lang::Expr>, ExprPair>::typeSwitch(
					e->getType(), ep);
		} else {
			read = false;
		}
	}

	return e;
}

std::shared_ptr<lang::Expr> WhileParser::readExprTerm(ParserContext &ctxt) {
	auto e = readExprTermInner(ctxt);
	auto tok = in.peek();

	bool read = true;
	while (read) {
		auto intype = e->getType();

		/*
		 * list lookups
		 */
		if (in.canMatch("[")) {// while to cover nested lists
			std::shared_ptr<lang::Type> inner_type;
			if (e->getType()->isList()) {
				lang::ListType &lt = (lang::ListType &) *intype;
				inner_type = lt.innerType();

			}
			else {
				inner_type = std::make_shared<lang::UnknownType>();
			}

			auto index = readExprAdd(ctxt);
			e = std::make_shared<lang::ListLookupExpr>(tok, e, inner_type, index);
			in.match("]");
			intype = inner_type;
		}

		/*
		 * record lookups
		 */
		else if (in.canMatch(".")) {	// record member
			auto memb = in.pop();
			std::shared_ptr<lang::Type> inner_type;
			if (e->getType()->isRecord()) {
				lang::RecordType &rt = (lang::RecordType &) *intype;
				inner_type = rt.memberType(memb.text()).type();
			}
			else {
				inner_type = std::make_shared<lang::UnknownType>();
			}
			e = std::make_shared<lang::RecordMemberExpr>(memb, e, inner_type, memb.text());
		}
		else {
			read = false;
		}
	}
	return e;
}

std::shared_ptr<lang::Expr> WhileParser::readExprTermInner(ParserContext &ctxt) {
	std::shared_ptr<lang::Expr> e;
	auto t = in.peek();

	if (in.canMatch("(")) {
		auto next = in.peek();

		/*
		 * primative casting
		 */
		if ( dectypes.count(next.text()) > 0 || next.text() == "{" || next.text() == "[" ) {
			auto type = readType();
			in.match(")");
			auto inner = readExpr(ctxt);
			e = std::make_shared<lang::BasicCastExpr>(t, type, inner);
		}

		/*
		 * parentheses on expr
		 */
		else {
			e = readExpr(ctxt);
			in.match(")");
		}
	}

	/*
	 *  const, variables and functions - only these begin with alphabetic character
	 *  rule out any const value first (which may not be alphabetic)
	 */
	else if (constvals.count( t.text() ) > 0) {
		e = (*constvals.find( t.text() )).second;
		in.pop();
	}
	else if ( isalpha(t.text()[0]) ) {
		auto name = in.pop();

		if (in.canMatch("(")) {
			std::vector<std::shared_ptr<lang::Expr>> args;
			if ( !in.canMatch(")") ) {
				args.push_back(readExpr(ctxt));
				while ( !in.canMatch(")") ) {
					in.match(",");
					args.push_back(readExpr(ctxt));
				}
			}
			auto i = functions.find( t.text() );
			if (i == functions.end()) {
				throw TokenException(name, "unordered function declarations not implemented");
				//e = shared_ptr<Expr>( new FuncCallExpr(shared_ptr<Func>(new Func((*i).second)), args) );
			}
			else {
				e = std::make_shared<lang::FuncCallExpr>( t, (*i).second, args );
			}
		}
		else if ( ctxt.isVar(t) ) {
			auto v = ctxt.copyVar(t);
			e = std::make_shared<lang::VariableExpr>(t, std::make_shared<lang::Var>(v));
		}
		else {
			e = std::make_shared<lang::VariableExpr>(t, unresolvedVar(name));
		}
	}
	else if (in.canMatch("[")) {
		std::vector<std::shared_ptr<lang::Expr>> list;
		if ( !in.canMatch("]") ) {
			list.push_back( readExpr(ctxt) );
			while ( !in.canMatch("]") ) {
				in.match(",");
				list.push_back( readExpr(ctxt) );
			}
		}
		e = std::make_shared<lang::ListExpr>(t, list);
	}

	/*
	 * string values
	 */
	else if (in.canMatch("\"")) {
		std::string s;
		if (in.canMatch("\"")) {
			s = "";
		}
		else {
			s = in.pop().text();
			in.match("\"");
		}
		auto i = dectypes.find("char");
		lang::ValueList values;
		for (char c: s) {
			values.push_back( std::make_shared<lang::TypedValue<char>>((*i).second, c) );
		}

		auto v = std::make_shared<lang::TypedValue<lang::ValueList>>(stringtype, values);
		e = std::make_shared<lang::ConstExpr>(t, v);
	}
	else if (in.canMatch("{")) {
		std::map<std::string, lang::Var> rec_type;
		std::map<std::string, std::shared_ptr<lang::Expr>> vars;

		if (!in.canMatch("}")) {
			auto name = in.pop();
			in.match(":");
			auto elem = readExpr(ctxt);
			rec_type.insert( {name.text(), lang::Var(elem->getType(), name)} );
			vars.insert( {name.text(), elem} );
			while (!in.canMatch("}")) {
				in.match(",");
				auto name = in.pop();
				in.match(":");
				auto elem = readExpr(ctxt);
				rec_type.insert( {name.text(), lang::Var(elem->getType(), name)} );
				vars.insert( {name.text(), elem} );
			}
		}

		auto type = std::make_shared<lang::RecordType>(rec_type);
		e = std::make_shared<lang::RecordExpr>(t, type, vars);
	}
	else {
		return readExprPrimary(ctxt);
	}
	return e;
}

std::shared_ptr<lang::Expr> WhileParser::readExprPrimary(ParserContext &ctxt) {
	std::shared_ptr<lang::Expr> e;
	auto t = in.peek();

	if (in.canMatch("|")) {
		auto second = readExprAdd(ctxt);
		in.match("|");
		e = std::make_shared<lang::ListLengthExpr>(t, second);
	}
	else if (in.canMatch("!")) {
		auto second = readExprCmpr(ctxt);
		e = std::make_shared<lang::NotExpr>(t, second);
	}
	else if (in.canMatch("-")) {
		auto inner = readExprTerm(ctxt);

		// create zero with matching type
		auto expzero = lang::TypeSwitch<ZeroParser, std::shared_ptr<lang::Expr>, std::shared_ptr<lang::Type>>::typeSwitch( inner->getType(), inner->getType() );
		ExprPair ep = ExprPair(expzero, inner);
		e = lang::TypeSwitch<SubParser, std::shared_ptr<lang::Expr>, ExprPair>::typeSwitch( inner->getType(), ep );
	}
	else {
		return readConstExpr();
	}

	// basic order of operation control
	return e;
}

std::shared_ptr<lang::Expr> WhileParser::readConstExpr() {
	std::shared_ptr<lang::Expr> e;
	auto t = in.peek();

	/*
	 * non-negative numrical values
	 */
	if ( isdigit(t.text()[0]) ) {
		if (t.contains('.')) {
			double double_value = atof(in.pop().text().c_str());
			auto v = std::make_shared<lang::TypedValue<double>>((*dectypes.find("real")).second, double_value);
			e = std::make_shared<lang::ConstExpr>(t, v);
		} else {
			int int_value = atoi(in.pop().text().c_str());
			auto v = std::make_shared<lang::TypedValue<int>>((*dectypes.find("int")).second, int_value);
			e = std::make_shared<lang::ConstExpr>(t, v);
		}
	}
	else if (in.canMatch("'")) {
		auto i = dectypes.find("char");

		// character constant
		auto v = std::make_shared<lang::TypedValue<char>>((*i).second, in.pop().text()[0]);
		e = std::make_shared<lang::ConstExpr>(t, v);
		in.match("'");
	}
	else {
		throw TokenException(t, "could not identify expression "+t.text());
	}

	return e;
}

std::shared_ptr<lang::Var> WhileParser::unresolvedVar(Token n) {
	auto t = std::make_shared<lang::UnknownType>();
	auto v = std::make_shared<lang::Var>(t, n);
	unresolved.push_back(v);
	return v;
}

std::shared_ptr<lang::Func> WhileParser::readFunc() {
	ParserContext ctxt;

	auto t = readType();

	Token name = in.pop(); // func name
	in.match("(");

	std::vector<lang::Var> args;
	if ( !in.canMatch(")") ) {
		auto type = readType();
		Token name = in.pop();
		ctxt.initialise(type, name);
		args.push_back(ctxt.copyVar(name));
		while ( !in.canMatch(")") ) {
			in.match(",");
			auto type = readType();
			Token name = in.pop();
			ctxt.initialise(type, name);
			args.push_back(ctxt.copyVar(name));
		}
	}

	return std::make_shared<lang::Func>( name.text(), t, args, readStmtBlock(ctxt) );
}

std::shared_ptr<lang::Stmt> WhileParser::readStmtBlock(ParserContext &ctxt) {
	if (in.canMatch("{")) {
		std::vector<std::shared_ptr<lang::Stmt>> stmts;
		while (!in.canMatch("}")) {
			stmts.push_back( readStmt(ctxt) );
		}
		return std::make_shared<lang::BlockStmt>(stmts);
	}
	else return readStmt(ctxt);
}

std::shared_ptr<lang::Stmt> WhileParser::readStmt(ParserContext &ctxt) {
	std::shared_ptr<lang::Stmt> stmt;
	auto top = in.peek();

	if (in.canMatch("if")) {
		in.match("(");
		auto e = readExpr(ctxt);
		in.match(")");
		auto list = readStmtBlock(ctxt);
		std::shared_ptr<lang::Stmt> alt;
		if (in.canMatch("else")) {
			alt = readStmtBlock(ctxt);
		}
		return std::make_shared<lang::IfStmt>(e, list, alt);
	}
	else if (in.canMatch("while")) {
		in.match("(");
		auto e = readExpr(ctxt);
		in.match(")");
		auto list = readStmtBlock(ctxt);
		return std::make_shared<lang::WhileStmt>(e, list);
	}
	else if (in.canMatch("for")) {
		std::shared_ptr<lang::Stmt> init;
		std::shared_ptr<lang::Expr> cond;
		std::shared_ptr<lang::Stmt> inc;
		in.match("(");
		if (!in.canMatch(";")) init = readStmt(ctxt);
		if (!in.canMatch(";")) {
			cond = readExpr(ctxt);
			in.match(";");
		}
		if (!in.canMatch(";")) inc = readVariableAssign(ctxt);
		in.match(")");
		auto list = readStmtBlock(ctxt);
		return std::make_shared<lang::ForStmt>(init, cond, inc, list);
	}
	else if (in.canMatch("switch")) {
		in.match("(");
		auto swt = readExpr(ctxt);
		in.match(")");
		in.match("{");
		std::map<std::shared_ptr<lang::Expr>, std::shared_ptr<lang::Stmt>> list;
		std::shared_ptr<lang::Stmt> def_stmt;
		while ( !in.canMatch("}") ) {
			if (in.canMatch("case")) {
				auto ch = readExpr(ctxt);
				in.match(":");
				std::vector<std::shared_ptr<lang::Stmt>> stmts;
				Token peek = in.peek();
				while (!(peek.text() == "case" || peek.text() == "default" || peek.text() == "}")) {
					stmts.push_back(readStmt(ctxt));
					peek = in.peek();
				}
				auto st = std::make_shared<lang::BlockStmt>(stmts);
				list.insert( {ch, st} );
			}
			else if (in.canMatch("default")) {
				in.match(":");
				std::vector<std::shared_ptr<lang::Stmt>> stmts;
				Token peek = in.peek();
				while (!(peek.text() == "case" || peek.text() == "}")) {
					stmts.push_back(readStmt(ctxt));
					peek = in.peek();
				}
				def_stmt = std::make_shared<lang::BlockStmt>(stmts);
			}
			else {
				throw std::runtime_error("could not identify a switch element from "+in.peek().text());
			}
		}
		return std::make_shared<lang::SwitchStmt>(swt, list, def_stmt);
	}
	else if (in.canMatch("return")) {
		if (in.canMatch(";")) {
			stmt = std::make_shared<lang::ReturnStmt>();
		}
		else {
			stmt = std::make_shared<lang::ReturnStmt>( readExpr(ctxt) );
		}
	}
	else if (in.canMatch("break")) {
		stmt = std::make_shared<lang::BreakStmt>();
	}
	else if (in.canMatch("print")) {
		stmt = std::make_shared<lang::PrintStmt>( readExpr(ctxt) );
	}
	else if (functions.count(top.text()) > 0) {
		stmt = std::make_shared<lang::EvalStmt>( readExpr(ctxt) );	// function calls
	}
	else {
		stmt = readVariableAssign(ctxt);
	}
	in.match(";");
	return stmt;
}

std::shared_ptr<lang::Stmt> WhileParser::readVariableAssign(ParserContext &ctxt) {
	auto top = in.peek();

	/*
	 * initial assignment.
	 * first reads a type
	 */
	if (dectypes.count(top.text()) > 0 || top.text() == "["
			|| top.text() == "{") {	// undeclared list types begin with [
		auto type = readType();
		Token name = in.pop();
		ctxt.initialise(type, name);
		auto v = ctxt.copyVar(name);
		if (in.canMatch("=")) { // ok to skip initial assignment
			auto e = readExpr(ctxt);
			return std::make_shared<lang::InitStmt>(v, e);
		} else {
			return std::make_shared<lang::InitStmt>(v);
		}
	}
	else {
		auto lhs = readExprTerm(ctxt);
		if ( in.canMatch("=") ) {
			return std::make_shared<lang::AssignStmt>(lhs, readExpr(ctxt));
		}
		else {
			return std::make_shared<lang::EvalStmt>(lhs);	// does nothing unless a function is called
		}
	}
}

std::shared_ptr<lang::Type> WhileParser::readType() {
	std::vector<std::shared_ptr<lang::Type>> types;
	types.push_back( readTypeInner() );

	// union types
	while (in.canMatch("|")) {
		types.push_back( readTypeInner() );
	}

	if (types.size() == 1) {
		return types[0];
	}
	else {
		return std::make_shared<lang::UnionType>(types);
	}
}

std::shared_ptr<lang::Type> WhileParser::readTypeInner() {
	std::shared_ptr<lang::Type> retType;
	string top = in.peek().text();

	if ( in.canMatch("[") ) {
		auto t = readType();
		in.match("]");
		retType = std::make_shared<lang::ListType>(t);
	}
	else if ( in.canMatch("{") ) {
		std::map<std::string, lang::Var> vars;

		if (!in.canMatch("}")) {
			auto t = readType();
			auto nm = in.pop();
			vars.insert( {nm.text(), lang::Var(t, nm)} );
		}
		while (!in.canMatch("}")) {
			in.match(",");
			auto t = readType();
			auto nm = in.pop();
			vars.insert( {nm.text(), lang::Var(t, nm)} );
		}
		retType = make_shared<lang::RecordType>( vars );
	}
	else if (dectypes.count(top) > 0) {
		in.pop();
		auto i = dectypes.find(top);
		retType = (*i).second;
	}
	else {
		throw std::runtime_error("could not identify a type from "+top);
	}
	return retType;
}

} /* namespace parser */
} /* namespace io */
