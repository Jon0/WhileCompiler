#include <iostream>
#include <stdexcept>

#include "Parser.h"

namespace std {

Parser::Parser(Lexer &lexer) :
		in(lexer) {
	size_t ind = lexer.getFilename().find(".");
	classname = lexer.getFilename().substr(0, ind);

	// initialise atomic types
	vector<shared_ptr<Type>> initial_types {
		shared_ptr<Type>(new VoidType()),
		shared_ptr<Type>(new NullType()),
		shared_ptr<Type>(new AtomicType<bool>("bool")),
		shared_ptr<Type>(new AtomicType<char>("char")),
		shared_ptr<Type>(new AtomicType<int>("int")),
		shared_ptr<Type>(new AtomicType<double>("real")),
	};

	for (shared_ptr<Type> p: initial_types) {
		dectypes.insert( map<string, shared_ptr<Type>>::value_type(p->nameStr(), p) );
	}

	/*
	 * define string as list of char
	 */
	stringtype = shared_ptr<Type>(new ListType( (*dectypes.find("char")).second, true ));
	dectypes.insert( map<string, shared_ptr<Type>>::value_type("string", stringtype) );

	/*
	 * some internal constants
	 */
	shared_ptr<Value> v1 = shared_ptr<Value>(new TypedValue<int>((*dectypes.find("int")).second, 0));
	intzero = shared_ptr<Expr>(new ConstExpr(v1));

	shared_ptr<Value> v2 = shared_ptr<Value>(new TypedValue<int>((*dectypes.find("real")).second, 0));
	realzero = shared_ptr<Expr>(new ConstExpr(v2));

	nulltype = (*dectypes.find("null")).second;
	booltype = (*dectypes.find("bool")).second;

	nullvalue = shared_ptr<Value>(new NullValue( nulltype ));
	truevalue = shared_ptr<Value>(new TypedValue<bool>( booltype, true ));
	falsevalue = shared_ptr<Value>(new TypedValue<bool>( booltype, false ));


	/*
	 * define null, true and false as constants
	 */
	constvals.insert( map<string, shared_ptr<Expr>>::value_type("null", shared_ptr<Expr>(new ConstExpr(nullvalue))) );
	constvals.insert( map<string, shared_ptr<Expr>>::value_type("true", shared_ptr<Expr>(new ConstExpr(truevalue))) );
	constvals.insert( map<string, shared_ptr<Expr>>::value_type("false", shared_ptr<Expr>(new ConstExpr(falsevalue))) );
}

Parser::~Parser() {
}

shared_ptr<Program> Parser::read() {
	try {
		while ( !in.empty() ) {
			if (in.canMatch("const")) {
				Token name = in.pop();
				in.match("is");
				ParserContext temp_ctxt;
				shared_ptr<Expr> assoc = readExpr(temp_ctxt);
				constvals.insert(map<string, shared_ptr<Expr>>::value_type(name.text(), assoc));
			}
			else if (in.canMatch("type")) {
				Token name = in.pop();
				in.match("is");
				shared_ptr<Type> type = readType();
				dectypes.insert(map<string, shared_ptr<Type>>::value_type(name.text(), type->makeAlias( name.text() )));
			}
			else {
				shared_ptr<Func> f = readFunc();
				functions.insert(FuncMap::value_type(f->name(), f));
			}
		}
	} catch (exception &e) {
		cout << e.what() << endl;
		throw runtime_error("failed to parse program");
	}

	return shared_ptr<Program>(new Program(functions, classname));
}

shared_ptr<Expr> Parser::readExpr(ParserContext &ctxt) {
	shared_ptr<Expr> e = readExprCmpr(ctxt);
	Token tok = in.peek();

	/*
	 * Boolean operations
	 */
	bool read = true;
	while (read) {
		if (in.canMatch("&")) {
			in.match("&"); // second &
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new AndExpr(tok, booltype, e, second));
		} else if (in.canMatch("|")) {
			in.match("|"); // second |
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OrExpr(tok, booltype, e, second));
		} else {
			read = false;
		}
	}
	return e;
}

shared_ptr<Expr> Parser::readExprCmpr(ParserContext &ctxt) {
	shared_ptr<Expr> e = readExprList(ctxt);
	Token tok = in.peek();

	/*
	 * Equality on any values
	 */
	if (in.canMatch("=")) {
		in.match("="); // second =
		shared_ptr<Expr> second = readExprList(ctxt);
		e = shared_ptr<Expr>(new EquivOp(tok, booltype, e, second));
	}
	else if (in.canMatch("!")) {
		in.match("="); // second =
		shared_ptr<Expr> second = readExprList(ctxt);
		e = shared_ptr<Expr>(new NotEquivOp(tok, booltype, e, second));
	}
	else if (in.canMatch(">")) {
		if (in.canMatch("=")) {
			ExprPair ep = ExprPair(e, readExprList(ctxt));
			e = TypeSwitch<GreaterEqualParser, shared_ptr<Expr>, ExprPair>::typeSwitch( e->getType(), ep );
		} else {
			ExprPair ep = ExprPair(e, readExprList(ctxt));
			e = TypeSwitch<GreaterParser, shared_ptr<Expr>, ExprPair>::typeSwitch( e->getType(), ep );
		}
	} else if (in.canMatch("<")) {
		if (in.canMatch("=")) {
			ExprPair ep = ExprPair(e, readExprList(ctxt));
			e = TypeSwitch<LessEqualParser, shared_ptr<Expr>, ExprPair>::typeSwitch( e->getType(), ep );
		} else {
			ExprPair ep = ExprPair(e, readExprList(ctxt));
			e = TypeSwitch<LessParser, shared_ptr<Expr>, ExprPair>::typeSwitch( e->getType(), ep );
		}
	}
	else if (in.canMatch("is")) {
		// type comparison
		shared_ptr<Type> ty = readType();
		e = shared_ptr<Expr>(new IsTypeExpr(tok, e, ty));
	}
	return e;
}

shared_ptr<Expr> Parser::readExprList(ParserContext &ctxt) {
	shared_ptr<Expr> e = readExprAdd(ctxt);
	Token tok = in.peek();

	// first expr is list (++ concat)
	if (in.canMatch("++")) {
		shared_ptr<Expr> second = readExprList(ctxt);
		e = shared_ptr<Expr>(new ConcatExpr(tok, e->getType(), e, second)); // TODO infer type from first term?
	}
	return e;
}

shared_ptr<Expr> Parser::readExprAdd(ParserContext &ctxt) {
	shared_ptr<Expr> e = readExprMul(ctxt);

	bool read = true;
	while (read) {
		if (in.canMatch("+")) {
			ExprPair ep = ExprPair(e, readExprMul(ctxt));
			e = TypeSwitch<AddParser, shared_ptr<Expr>, ExprPair>::typeSwitch(
					e->getType(), ep);
		} else if (in.canMatch("-")) {
			ExprPair ep = ExprPair(e, readExprMul(ctxt));
			e = TypeSwitch<SubParser, shared_ptr<Expr>, ExprPair>::typeSwitch(
					e->getType(), ep);
		} else {
			read = false;
		}
	}

	return e;
}

shared_ptr<Expr> Parser::readExprMul(ParserContext &ctxt) {
	shared_ptr<Expr> e = readExprTerm(ctxt);

	/*
	 * Math Operations
	 */
	bool read = true;
	while (read) {
		if (in.canMatch("*")) {
			ExprPair ep = ExprPair(e, readExprTerm(ctxt));
			e = TypeSwitch<MulParser, shared_ptr<Expr>, ExprPair>::typeSwitch(
					e->getType(), ep);
		} else if (in.canMatch("/")) {
			ExprPair ep = ExprPair(e, readExprTerm(ctxt));
			e = TypeSwitch<DivParser, shared_ptr<Expr>, ExprPair>::typeSwitch(
					e->getType(), ep);
		} else if (in.canMatch("%")) {
			ExprPair ep = ExprPair(e, readExprTerm(ctxt));
			e = TypeSwitch<ModParser, shared_ptr<Expr>, ExprPair>::typeSwitch(
					e->getType(), ep);
		} else {
			read = false;
		}
	}

	return e;
}

shared_ptr<Expr> Parser::readExprTerm(ParserContext &ctxt) {
	shared_ptr<Expr> e = readExprTermInner(ctxt);
	Token tok = in.peek();

	bool read = true;
	while (read) {
		shared_ptr<Type> intype = e->getType();

		/*
		 * list lookups
		 */
		if (in.canMatch("[")) {// while to cover nested lists
			shared_ptr<Type> inner_type;
			if (e->getType()->isList()) {
				ListType &lt = (ListType &) *intype;
				inner_type = lt.innerType();

			}
			else {
				inner_type = shared_ptr<Type>(new UnknownType());
			}

			shared_ptr<Expr> index = readExprAdd(ctxt);
			e = shared_ptr<Expr>(new ListLookupExpr(tok, e, inner_type, index));
			in.match("]");
			intype = inner_type;
		}

		/*
		 * record lookups
		 */
		else if (in.canMatch(".")) {	// record member
			Token memb = in.pop();
			shared_ptr<Type> inner_type;
			if (e->getType()->isRecord()) {
				RecordType &rt = (RecordType &) *intype;
				inner_type = rt.memberType(memb.text()).type();
			}
			else {
				inner_type = shared_ptr<Type>(new UnknownType());
			}
			e = shared_ptr<Expr>(new RecordMemberExpr(memb, e, inner_type, memb.text()));
		}
		else {
			read = false;
		}
	}
	return e;
}

shared_ptr<Expr> Parser::readExprTermInner(ParserContext &ctxt) {
	shared_ptr<Expr> e;
	Token t = in.peek();

	if (in.canMatch("(")) {
		Token next = in.peek();

		/*
		 * primative casting
		 */
		if ( dectypes.count(next.text()) > 0 || next.text() == "{" || next.text() == "[" ) {
			shared_ptr<Type> type = readType();
			in.match(")");
			shared_ptr<Expr> inner = readExpr(ctxt);
			e = shared_ptr<Expr>(new BasicCastExpr(t, type, inner));
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
		Token name = in.pop();

		if (in.canMatch("(")) {
			vector<shared_ptr<Expr>> args;
			if ( !in.canMatch(")") ) {
				args.push_back(readExpr(ctxt));
				while ( !in.canMatch(")") ) {
					in.match(",");
					args.push_back(readExpr(ctxt));
				}
			}
			FuncMap::iterator i = functions.find( t.text() );
			if (i == functions.end()) {
				throw TokenException(name, "unordered function declarations not implemented");
				//e = shared_ptr<Expr>( new FuncCallExpr(shared_ptr<Func>(new Func((*i).second)), args) );
			}
			else {
				e = shared_ptr<Expr>( new FuncCallExpr(t, (*i).second, args) );
			}
		}
		else if ( ctxt.isVar(t) ) {
			Var v = ctxt.copyVar(t);
			e = shared_ptr<Expr>( new VariableExpr( t, shared_ptr<Var>(new Var(v) ) ) );
		}
		else {
			e = shared_ptr<Expr>( new VariableExpr( t, unresolvedVar(name) ) );
		}
	}
	else if (in.canMatch("[")) {
		vector<shared_ptr<Expr>> list;
		if ( !in.canMatch("]") ) {
			list.push_back( readExpr(ctxt) );
			while ( !in.canMatch("]") ) {
				in.match(",");
				list.push_back( readExpr(ctxt) );
			}
		}
		e = shared_ptr<Expr>(new ListExpr( t, list ));
	}

	/*
	 * string values
	 */
	else if (in.canMatch("\"")) {
		string s;
		if (in.canMatch("\"")) {
			s = "";
		}
		else {
			s = in.pop().text();
			in.match("\"");
		}
		map<string, shared_ptr<Type>>::iterator i = dectypes.find("char");
		ValueList values;
		for (char c: s) {
			values.push_back( shared_ptr<Value>(new TypedValue<char>( (*i).second, c )) );
		}

		shared_ptr<Value> v = shared_ptr<Value>(new TypedValue<ValueList>( stringtype, values ));
		e = shared_ptr<Expr>(new ConstExpr(t, v));
	}
	else if (in.canMatch("{")) {
		map<string, Var> rec_type;
		map<string, shared_ptr<Expr>> vars;

		if (!in.canMatch("}")) {
			Token name = in.pop();
			in.match(":");
			shared_ptr<Expr> elem = readExpr(ctxt);
			rec_type.insert( map<string, Var>::value_type(name.text(), Var(elem->getType(), name)) );
			vars.insert( map<string, shared_ptr<Expr>>::value_type(name.text(), elem) );
			while (!in.canMatch("}")) {
				in.match(",");
				Token name = in.pop();
				in.match(":");
				shared_ptr<Expr> elem = readExpr(ctxt);
				rec_type.insert( map<string, Var>::value_type(name.text(), Var(elem->getType(), name)) );
				vars.insert( map<string, shared_ptr<Expr>>::value_type(name.text(), elem) );
			}
		}

		shared_ptr<Type> type = shared_ptr<Type>( new RecordType(rec_type) );
		e = shared_ptr<Expr>( new RecordExpr( t, type, vars) );
	}
	else {
		return readExprPrimary(ctxt);
	}
	return e;
}

shared_ptr<Expr> Parser::readExprPrimary(ParserContext &ctxt) {
	Token t = in.peek();
	shared_ptr<Expr> e;


	if (in.canMatch("|")) {
		shared_ptr<Expr> exp = readExprAdd(ctxt);
		in.match("|");
		e = shared_ptr<Expr>(new ListLengthExpr( t, exp));
	}
	else if (in.canMatch("!")) {
		shared_ptr<Expr> second = readExprCmpr(ctxt);
		e = shared_ptr<Expr>(new NotExpr( t, second));
	}
	else if (in.canMatch("-")) {
		shared_ptr<Expr> inner = readExprTerm(ctxt);

		// create zero with matching type
		shared_ptr<Expr> expzero = TypeSwitch<ZeroParser, shared_ptr<Expr>, shared_ptr<Type>>::typeSwitch( inner->getType(), inner->getType() );
		ExprPair ep = ExprPair(expzero, inner);
		e = TypeSwitch<SubParser, shared_ptr<Expr>, ExprPair>::typeSwitch( inner->getType(), ep );
	}
	else {
		return readConstExpr();
	}

	// basic order of operation control
	return e;
}

shared_ptr<Expr> Parser::readConstExpr() {
	Token t = in.peek();
	shared_ptr<Expr> e;

	/*
	 * non-negative numrical values
	 */
	if ( isdigit(t.text()[0]) ) {
		if (t.contains('.')) {
			double int_value = atof(in.pop().text().c_str());
			shared_ptr<Value> v = shared_ptr<Value>(
					new TypedValue<double>((*dectypes.find("real")).second, int_value));
			e = shared_ptr<Expr>(new ConstExpr(t, v));
		} else {
			int int_value = atoi(in.pop().text().c_str());
			shared_ptr<Value> v = shared_ptr<Value>(
					new TypedValue<int>((*dectypes.find("int")).second, int_value));
			e = shared_ptr<Expr>(new ConstExpr(t, v));
		}
	}
	else if (in.canMatch("'")) {
		map<string, shared_ptr<Type>>::iterator i = dectypes.find("char");

		shared_ptr<Value> v = shared_ptr<Value>(
				new TypedValue<char>( (*i).second, in.pop().text()[0]));
		e = shared_ptr<Expr>(new ConstExpr(t, v));
		in.match("'");
	}
	else {
		throw TokenException(t, "could not identify expression "+t.text());
	}

	return e;
}

shared_ptr<Var> Parser::unresolvedVar(Token n) {
	shared_ptr<Type> t = shared_ptr<Type>(new UnknownType());
	shared_ptr<Var> v = shared_ptr<Var>(new Var(t, n));
	unresolved.push_back(v);
	return v;
}

shared_ptr<Func> Parser::readFunc() {
	ParserContext ctxt;

	shared_ptr<Type> t = readType();

	Token name = in.pop(); // func name
	in.match("(");

	vector<Var> args;
	if ( !in.canMatch(")") ) {
		shared_ptr<Type> type = readType();
		Token name = in.pop();
		ctxt.initialise(type, name);
		args.push_back(ctxt.copyVar(name));
		while ( !in.canMatch(")") ) {
			in.match(",");
			shared_ptr<Type> type = readType();
			Token name = in.pop();
			ctxt.initialise(type, name);
			args.push_back(ctxt.copyVar(name));
		}
	}

	return shared_ptr<Func>(new Func( name.text(), t, args, readStmtBlock(ctxt) ));
}

shared_ptr<Stmt> Parser::readStmtBlock(ParserContext &ctxt) {
	if (in.canMatch("{")) {
		vector<shared_ptr<Stmt>> stmts;
		while (!in.canMatch("}")) {
			stmts.push_back( readStmt(ctxt) );
		}
		return shared_ptr<Stmt>(new BlockStmt( stmts ));
	}
	else return readStmt(ctxt);
}

shared_ptr<Stmt> Parser::readStmt(ParserContext &ctxt) {
	shared_ptr<Stmt> stmt;
	Token top = in.peek();

	if (in.canMatch("if")) {
		in.match("(");
		shared_ptr<Expr> e = readExpr(ctxt);
		in.match(")");
		shared_ptr<Stmt> list = readStmtBlock(ctxt);
		shared_ptr<Stmt> alt;
		if (in.canMatch("else")) {
			alt = readStmtBlock(ctxt);
		}
		return shared_ptr<Stmt>(new IfStmt(e, list, alt));
	}
	else if (in.canMatch("while")) {
		in.match("(");
		shared_ptr<Expr> e = readExpr(ctxt);
		in.match(")");
		shared_ptr<Stmt> list = readStmtBlock(ctxt);
		return shared_ptr<Stmt>(new WhileStmt(e, list));
	}
	else if (in.canMatch("for")) {
		shared_ptr<Stmt> init;
		shared_ptr<Expr> cond;
		shared_ptr<Stmt> inc;
		in.match("(");
		if (!in.canMatch(";")) init = readStmt(ctxt);
		if (!in.canMatch(";")) {
			cond = readExpr(ctxt);
			in.match(";");
		}
		if (!in.canMatch(";")) inc = readVariableAssign(ctxt);
		in.match(")");
		shared_ptr<Stmt> list = readStmtBlock(ctxt);
		return shared_ptr<Stmt>(new ForStmt(init, cond, inc, list));
	}
	else if (in.canMatch("switch")) {
		in.match("(");
		shared_ptr<Expr> swt = readExpr(ctxt);
		in.match(")");
		in.match("{");
		map<shared_ptr<Expr>, shared_ptr<Stmt>> list;
		shared_ptr<Stmt> def_stmt;
		while ( !in.canMatch("}") ) {
			if (in.canMatch("case")) {
				shared_ptr<Expr> ch = readExpr(ctxt);
				in.match(":");
				vector<shared_ptr<Stmt>> stmts;
				Token peek = in.peek();
				while (!(peek.text() == "case" || peek.text() == "default" || peek.text() == "}")) {
					stmts.push_back(readStmt(ctxt));
					peek = in.peek();
				}
				shared_ptr<Stmt> st = shared_ptr<Stmt>(new BlockStmt(stmts));

				list.insert(map<shared_ptr<Expr>, shared_ptr<Stmt>>::value_type(ch,st));
			}
			else if (in.canMatch("default")) {
				in.match(":");
				vector<shared_ptr<Stmt>> stmts;
				Token peek = in.peek();
				while (!(peek.text() == "case" || peek.text() == "}")) {
					stmts.push_back(readStmt(ctxt));
					peek = in.peek();
				}
				def_stmt = shared_ptr<Stmt>(new BlockStmt(stmts));
			}
			else {
				throw runtime_error("could not identify a switch element from "+in.peek().text());
			}
		}
		return shared_ptr<Stmt>(new SwitchStmt(swt, list, def_stmt));
	}
	else if (in.canMatch("return")) {
		if (in.canMatch(";")) {
			return shared_ptr<Stmt>(new ReturnStmt());
		}
		else {
			stmt = shared_ptr<Stmt>(new ReturnStmt( readExpr(ctxt) ));
		}
	}
	else if (in.canMatch("break")) {
		stmt = shared_ptr<Stmt>(new BreakStmt());
	}
	else if (in.canMatch("print")) {
		stmt = shared_ptr<Stmt>(new PrintStmt(readExpr(ctxt)));
	}
	else if (functions.count(top.text()) > 0) {
		stmt = shared_ptr<Stmt>(new EvalStmt( readExpr(ctxt) ));	// function calls
	}
	else {
		stmt = readVariableAssign(ctxt);
	}
	in.match(";");
	return stmt;
}

shared_ptr<Stmt> Parser::readVariableAssign(ParserContext &ctxt) {
	Token top = in.peek();

	/*
	 * initial assignment.
	 * first reads a type
	 */
	if (dectypes.count(top.text()) > 0 || top.text() == "["
			|| top.text() == "{") {	// undeclared list types begin with [
		shared_ptr<Type> type = readType();
		Token name = in.pop();
		ctxt.initialise(type, name);
		Var v = ctxt.copyVar(name);
		if (in.canMatch("=")) { // ok to skip initial assignment
			shared_ptr<Expr> e = readExpr(ctxt);
			return shared_ptr<Stmt>(new InitStmt(v, e));
		} else {
			return shared_ptr<Stmt>(new InitStmt(v));
		}
	}
	else {
		shared_ptr<Expr> lhs = readExprTerm(ctxt);
		if ( in.canMatch("=") ) {
			return shared_ptr<Stmt>(new AssignStmt(lhs, readExpr(ctxt)));
		}
		else {
			return shared_ptr<Stmt>(new EvalStmt( lhs ));	// does nothing
		}
	}

}

shared_ptr<Type> Parser::readType() {
	vector<shared_ptr<Type>> types;
	types.push_back( readTypeInner() );

	// union types
	while (in.canMatch("|")) {
		types.push_back( readTypeInner() );
	}

	if (types.size() == 1) {
		return types[0];
	}
	else {
		return shared_ptr<Type>( new UnionType(types) );
	}
}

shared_ptr<Type> Parser::readTypeInner() {
	string top = in.peek().text();
	shared_ptr<Type> retType;

	if ( in.canMatch("[") ) {
		shared_ptr<Type> t = readType();
		in.match("]");
		retType = shared_ptr<Type>( new ListType(t) );
	}
	else if ( in.canMatch("{") ) {
		map<string, Var> vars;

		if (!in.canMatch("}")) {
			shared_ptr<Type> t = readType();
			Token nm = in.pop();
			vars.insert( map<string, Var>::value_type(nm.text(), Var(t, nm)) );
		}
		while (!in.canMatch("}")) {
			in.match(",");
			shared_ptr<Type> t = readType();
			Token nm = in.pop();
			vars.insert( map<string, Var>::value_type(nm.text(), Var(t, nm)) );
		}
		retType = shared_ptr<Type>( new RecordType(vars) );
	}
	else if (dectypes.count(top) > 0) {
		in.pop();
		map<string, shared_ptr<Type>>::iterator i = dectypes.find(top);
		retType = (*i).second;
	}
	else {
		throw runtime_error("could not identify a type from "+top);
	}
	return retType;
}

} /* namespace std */
