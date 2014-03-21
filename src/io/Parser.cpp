#include <iostream>
#include <stdexcept>

#include "Parser.h"

namespace std {

// TODO template switch case over types, to match stirng name

Parser::Parser(Lexer &lexer) :
		in(lexer) {

	// TODO template value tpyes or map string to class
	// make type list static? lookup by sting

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
	shared_ptr<Type> s_type = shared_ptr<Type>(new ListType( (*dectypes.find("char")).second ));
	dectypes.insert( map<string, shared_ptr<Type>>::value_type("string", s_type) );
}

Parser::~Parser() {
}

Program Parser::read() {
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
				dectypes.insert(map<string, shared_ptr<Type>>::value_type(name.text(), type));
			}
			else {
				Func f = readFunc();
				functions.insert(FuncMap::value_type(f.name(), f));
			}
		}
	} catch (exception &e) {
		cerr << e.what() << endl;
		throw runtime_error("failed to parse program");
	}

	Program p(functions);
	return p;
}

shared_ptr<Expr> Parser::readConstExpr() {
	Token t = in.peek();
	shared_ptr<Expr> e;

	if (t.text() == "true") {
		in.pop();
		map<string, shared_ptr<Type>>::iterator i = dectypes.find("bool");

		shared_ptr<Value> v = shared_ptr<Value>(
				new TypedValue<bool>( (*i).second, true ));
		e = shared_ptr<Expr>(new ConstExpr(v));
	}
	else if (t.text() == "false") {
		in.pop();
		map<string, shared_ptr<Type>>::iterator i = dectypes.find("bool");

		shared_ptr<Value> v = shared_ptr<Value>(
				new TypedValue<bool>( (*i).second, false ));
		e = shared_ptr<Expr>(new ConstExpr(v));
	}
	else if (t.text() == "null") {
		in.pop();
		map<string, shared_ptr<Type>>::iterator i = dectypes.find("null");

		shared_ptr<Value> v = shared_ptr<Value>(new NullValue( (*i).second ));
		e = shared_ptr<Expr>(new ConstExpr(v));
	}

	/*
	 * non-negative numrical values
	 */
	else if ( isdigit(t.text()[0]) || t.text() == "-" ) {
		if (t.contains('.')) {
			double int_value = atof(in.pop().text().c_str());
			shared_ptr<Value> v = shared_ptr<Value>(
					new TypedValue<double>((*dectypes.find("real")).second, int_value));
			e = shared_ptr<Expr>(new ConstExpr(v));
		} else {
			int int_value = atoi(in.pop().text().c_str());
			shared_ptr<Value> v = shared_ptr<Value>(
					new TypedValue<int>((*dectypes.find("int")).second, int_value));
			e = shared_ptr<Expr>(new ConstExpr(v));
		}
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

		shared_ptr<Type> ltype = shared_ptr<Type>( new ListType((*i).second) );
		shared_ptr<Value> v = shared_ptr<Value>(new TypedValue<ValueList>( ltype, values ));
		e = shared_ptr<Expr>(new ConstExpr(v));
	}
	else if (in.canMatch("'")) {
		map<string, shared_ptr<Type>>::iterator i = dectypes.find("char");

		shared_ptr<Value> v = shared_ptr<Value>(
				new TypedValue<char>( (*i).second, in.pop().text()[0]));
		e = shared_ptr<Expr>(new ConstExpr(v));
		in.match("'");
	}

	return e;
}

shared_ptr<Expr> Parser::readExpr(ParserContext &ctxt) {
	Token t = in.peek();
	shared_ptr<Expr> e;
	//cout << "expr " << t.text() << endl;

	if (in.canMatch("(")) {
		Token next = in.peek();

		/*
		 * primative casting
		 */
		if ( dectypes.count(next.text()) > 0 || next.text() == "{" || next.text() == "[" ) {
			shared_ptr<Type> type = readType();
			in.match(")");
			shared_ptr<Expr> inner = readExpr(ctxt);

			if (type->isList()) {
				e = shared_ptr<Expr>( new BasicCastExpr(type, inner) );
			}
			else if (type->nameStr() == "real" && inner->getType()->nameStr() == "int")
				e = shared_ptr<Expr>(new CastExpr<double, int>(type, inner));
			else if (type->nameStr() == "int" && inner->getType()->nameStr() == "real")
				e = shared_ptr<Expr>(new CastExpr<int, double>(type, inner));
			else if (inner->getType()->contains(*type)) {
				e = shared_ptr<Expr>(new BasicCastExpr(type, inner));
			}
			else {
				throw TokenException(t, type->nameStr()+" to "+inner->getType()->nameStr()+" casting not supported");
			}
		}

		/*
		 * parentheses on expr
		 */
		else {
			e = readExpr(ctxt);
			in.match(")");
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

		shared_ptr<Type> type;
		if (list.empty()) {
			type = shared_ptr<Type>( new ListType() );
		}
		else {
			type = shared_ptr<Type>( new ListType( list[0]->getType() ) );
		}
		e = shared_ptr<Expr>(new ListExpr(type, list));
	}
	else if (in.canMatch("|")) {
		//Token var = in.pop();
		//Var v = ctxt.copyVar( var ); // TODO: could have expresion inside?
		shared_ptr<Expr> exp = readExpr(ctxt);
		in.match("|");

		if (!exp->getType()->isList()) {
			throw TokenException(t, "cannot get length of non list");
		}
		e = shared_ptr<Expr>(new ListLengthExpr(exp));
	}
	else if (in.canMatch("{")) {
		map<string, Var> rec_type;
		map<string, shared_ptr<Expr>> vars;

		if (!in.canMatch("}")) {
			string name = in.pop().text();
			in.match(":");
			shared_ptr<Expr> elem = readExpr(ctxt);
			rec_type.insert( map<string, Var>::value_type(name, Var(elem->getType(), name)) );
			vars.insert( map<string, shared_ptr<Expr>>::value_type(name, elem) );
			while (!in.canMatch("}")) {
				in.match(",");
				string name = in.pop().text();
				in.match(":");
				shared_ptr<Expr> elem = readExpr(ctxt);
				rec_type.insert( map<string, Var>::value_type(name, Var(elem->getType(), name)) );
				vars.insert( map<string, shared_ptr<Expr>>::value_type(name, elem) );
			}
		}

		shared_ptr<Type> type = shared_ptr<Type>( new RecordType(rec_type) );
		e = shared_ptr<Expr>( new RecordExpr(type, vars) );
	}
	else if ( isdigit(t.text()[0]) || t.text() == "'" || t.text() == "\"" || t.text() == "true" || t.text() == "false" || t.text() == "null" ) {
		e = readConstExpr();
	}

	/*
	 * evaluate variable types
	 */
	else if (ctxt.isVar(t)) {
		Var v = ctxt.copyVar(t);
		e = shared_ptr<Expr>(new VariableExpr(v));
		in.pop();
	}
	else if (functions.count( t.text() ) > 0) {
		in.pop(); // name
		in.match("(");

		FuncMap::iterator i = functions.find( t.text() );
		vector<shared_ptr<Expr>> args;
		if ( !in.canMatch(")") ) {
			args.push_back(readExpr(ctxt));
			while ( !in.canMatch(")") ) {
				in.match(",");
				args.push_back(readExpr(ctxt));
			}
		}
		e = shared_ptr<Expr>( new FuncCallExpr(shared_ptr<Func>(new Func((*i).second)), args) );
	}

	else if (in.canMatch("!")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new NotExpr(second));
	}
	else if (in.canMatch("-")) {	// TODO -real
		shared_ptr<Value> v = shared_ptr<Value>(
				new TypedValue<int>((*dectypes.find("int")).second, 0));
		shared_ptr<Expr> zero = shared_ptr<Expr>(new ConstExpr(v));
		shared_ptr<Expr> expr = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<int, int, SubOp<int>>(zero, expr));
	}
	else if (constvals.count( t.text() ) > 0) {
		e = (*constvals.find( t.text() )).second;
		in.pop();
	}
	else {
		throw TokenException(t, "could not identify expression "+t.text());
	}

	// -----------------------------------
	// optional continuation of expression
	// -----------------------------------

	/*
	 * list lookups
	 */
	shared_ptr<Type> intype = e->getType();
	while (intype->isList() && in.canMatch("[")) {	// while to cover nested lists
		ListType &lt = (ListType &)*intype;
		shared_ptr<Type> inner_type = lt.innerType();
		shared_ptr<Expr> index = readExpr(ctxt);
		e = shared_ptr<Expr>(new ListLookupExpr(e, inner_type, index));
		in.match("]");
		intype = inner_type;
	}
	if (!intype->isList() && in.canMatch("[")) {
		throw TokenException(t, "cannot get elements of non list types");
	}

	/*
	 * record lookups
	 */
	if (in.canMatch(".")) {	// record member
		if (!intype->isRecord()) {
			throw TokenException(t, "cannot get members of non record types");
		}

		string memb = in.pop().text();
		RecordType &rt = (RecordType &) *intype;
		shared_ptr<Type> inner_type = rt.memberType(memb).type();
		e = shared_ptr<Expr>(new RecordMemberExpr(e, inner_type, memb)); // TODO e thrown away in record case
	}

	// type comparison
	if (in.canMatch("is")) {
		shared_ptr<Type> ty = readType();
		e = shared_ptr<Expr>(new IsTypeExpr( e, ty ));
	}

	// first expr is list (++ concat)
	else if (e->getType()->isList() && in.canMatch("+")) {
		in.match("+"); // second +
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new ConcatExpr( e->getType(), e, second )); // TODO infer type from first term?
	}

	// first expr is bool
	else if (e->getType()->nameStr() == "bool" && in.canMatch("&")) {
		in.match("&"); // second &

		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, bool, AndOp>(e, second));
	}
	else if (e->getType()->nameStr() == "bool" && in.canMatch("=")) {
		in.match("="); // second =
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, bool, EquivOp<bool>>(e, second));
	}
	else if (e->getType()->nameStr() == "bool" && in.canMatch("!")) {
		in.match("="); // second =
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, bool, NotEquivOp<bool>>(e, second));
	}

	// first expr is char
	else if ( e->getType()->nameStr() == "char" && in.canMatch("+")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<char, char, AddOp<char>>(e, second));
	}
	else if ( e->getType()->nameStr() == "char" && in.canMatch("-")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<char, char, SubOp<char>>(e, second));
	}
	else if ( e->getType()->nameStr() == "char" && in.canMatch("*")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<char, char, MulOp<char>>(e, second));
	}
	else if ( e->getType()->nameStr() == "char" && in.canMatch("/")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<char, char, DivOp<char>>(e, second));
	}
	else if ( e->getType()->nameStr() == "char" && in.canMatch("=") ) {
		in.match("="); // second =
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, char, EquivOp<char>>((*dectypes.find("bool")).second, e, second));
	}
	else if ( e->getType()->nameStr() == "char" && in.canMatch("!") ) {
		in.match("="); // second =
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, char, NotEquivOp<char>>((*dectypes.find("bool")).second, e, second));
	}

	// first expr is int
	// TODO order of operations
	else if ( e->getType()->nameStr() == "int" && in.canMatch("+")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<int, int, AddOp<int>>(e, second));
	}
	else if ( e->getType()->nameStr() == "int" && in.canMatch("-")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<int, int, SubOp<int>>(e, second));
	}
	else if ( e->getType()->nameStr() == "int" && in.canMatch("*")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<int, int, MulOp<int>>(e, second));
	}
	else if ( e->getType()->nameStr() == "int" && in.canMatch("/")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<int, int, DivOp<int>>(e, second));
	}
	else if ( e->getType()->nameStr() == "int" && in.canMatch("%")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<int, int, ModOp<int>>(e, second));
	}
	else if ( e->getType()->nameStr() == "int" && in.canMatch("=") ) {
		in.match("="); // second =
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, int, EquivOp<int>>((*dectypes.find("bool")).second, e, second));
	}
	else if ( e->getType()->nameStr() == "int" && in.canMatch("!") ) {
		in.match("="); // second =
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, int, NotEquivOp<int>>((*dectypes.find("bool")).second, e, second));
	}
	else if ( e->getType()->nameStr() == "int" && in.canMatch(">")) {
		if (in.canMatch("=")) {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, int, GreaterEqualOp<int>>((*dectypes.find("bool")).second, e, second));
		}
		else {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, int, GreaterOp<int>>((*dectypes.find("bool")).second, e, second));
		}
	}
	else if ( e->getType()->nameStr() == "int" && in.canMatch("<")) {
		if (in.canMatch("=")) {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, int, LessEqualOp<int>>((*dectypes.find("bool")).second, e, second));
		}
		else {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, int, LessOp<int>>((*dectypes.find("bool")).second, e, second));
		}
	}

	// first expr is real
	else if ( e->getType()->nameStr() == "real" && in.canMatch("+")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<double, double, AddOp<double>>(e, second));
	}
	else if ( e->getType()->nameStr() == "real" && in.canMatch("-")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<double, double, SubOp<double>>(e, second));
	}
	else if ( e->getType()->nameStr() == "real" && in.canMatch("*")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<double, double, MulOp<double>>(e, second));
	}
	else if ( e->getType()->nameStr() == "real" && in.canMatch("/")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<double, double, DivOp<double>>(e, second));
	}
	else if ( e->getType()->nameStr() == "real" && in.canMatch("=") ) {
		in.match("="); // second =

		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, double, EquivOp<double>>((*dectypes.find("bool")).second, e, second));
	}
	else if ( e->getType()->nameStr() == "real" && in.canMatch("!") ) {
		in.match("="); // second =

		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, double, NotEquivOp<double>>((*dectypes.find("bool")).second, e, second));
	}
	else if ( e->getType()->nameStr() == "real" && in.canMatch(">")) {
		if (in.canMatch("=")) {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, double, GreaterEqualOp<double>>((*dectypes.find("bool")).second, e, second));
		}
		else {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, double, GreaterOp<double>>((*dectypes.find("bool")).second, e, second));
		}
	}
	else if ( e->getType()->nameStr() == "real" && in.canMatch("<")) {
		if (in.canMatch("=")) {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, double, LessEqualOp<double>>((*dectypes.find("bool")).second, e, second));
		}
		else {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, double, LessOp<double>>((*dectypes.find("bool")).second, e, second));
		}
	}

	return e;
}

Func Parser::readFunc() {
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

	Func f( name.text(), t, args, readStmt(ctxt) );
	return f;
}

shared_ptr<Stmt> Parser::readStmt(ParserContext &ctxt) {
	shared_ptr<Stmt> stmt;
	Token top = in.peek();

	//cout << "stmt " << top.text() << endl;

	if (in.canMatch("{")) {
		vector<shared_ptr<Stmt>> stmts;
		while (!in.canMatch("}")) {
			stmts.push_back( readStmt(ctxt) );
		}
		return shared_ptr<Stmt>(new BlockStmt( stmts )); // no ; after
	}
	// variables
	else if ( dectypes.count( top.text() ) > 0 || top.text() == "[" || ctxt.isVar(top) ) {	// undeclared list types begin with [
		stmt = readVariableAssign(ctxt);
	}
	else if (in.canMatch("if")) {
		in.match("(");
		shared_ptr<Expr> e = readExpr(ctxt);
		in.match(")");
		shared_ptr<Stmt> list = readStmt(ctxt);
		shared_ptr<Stmt> alt;
		if (in.canMatch("else")) {
			alt = readStmt(ctxt);
		}
		return shared_ptr<Stmt>(new IfStmt(e, list, alt));
	}
	else if (in.canMatch("while")) {
		in.match("(");
		shared_ptr<Expr> e = readExpr(ctxt);
		in.match(")");
		shared_ptr<Stmt> list = readStmt(ctxt);
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
		shared_ptr<Stmt> list = readStmt(ctxt);
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
	else {
		stmt = shared_ptr<Stmt>(new EvalStmt(readExpr(ctxt)));	// last option, try read an expr
	}
	in.match(";");
	return stmt;

	// TODO assign vars moved here
}

shared_ptr<Stmt> Parser::readVariableAssign(ParserContext &ctxt) {
	Token top = in.peek();
	if ( dectypes.count( top.text() ) > 0 || top.text() == "[" ) {	// undeclared list types begin with [
		shared_ptr<Type> type = readType();
		Token name = in.pop();
		ctxt.initialise(type, name);
		Var v = ctxt.copyVar(name);
		if (in.canMatch("=")) { // ok to skip initial assignment
			shared_ptr<Expr> e = readExpr(ctxt);
			if ( !type->contains(*e->getType()) ) {
				throw TokenException(top, type->nameStr() + " cannot be assigned with "+ e->getType()->nameStr());
			}
			return shared_ptr<Stmt>(new InitStmt(v, e));
		}
		else {
			return shared_ptr<Stmt>(new InitStmt(v));
		}
	}
	// assignment
	else if ( ctxt.isVar(top) ) {
		Var v = ctxt.copyVar(top);
		in.pop();

		if (in.canMatch("[")) {
			ListType &lt = (ListType &)*v.type();
			shared_ptr<Type> inner_type = lt.innerType();
			shared_ptr<Expr> index = readExpr(ctxt);
			in.match("]");
			in.match("=");
			shared_ptr<Expr> e = readExpr(ctxt);
			if ( !inner_type->contains(*e->getType()) ) {
				throw TokenException(top, v.type()->nameStr()+" cannot be assigned with "+e->getType()->nameStr());
			}
			return shared_ptr<Stmt>(new ListAssignStmt(v, e, index));
		}
		else if (in.canMatch(".")) {
			string member = in.pop().text();
			RecordType &rt = (RecordType &)*v.type();
			shared_ptr<Type> member_type = rt.memberType(member).type();

			in.match("=");
			shared_ptr<Expr> e = readExpr(ctxt);
			if ( !member_type->contains(*e->getType()) ) {
				throw TokenException(top, member_type->nameStr()+" cannot be assigned with "+e->getType()->nameStr());
			}
			return shared_ptr<Stmt>(new RecordAssignStmt(v, e, member));
		}
		else {
			in.match("=");
			shared_ptr<Expr> e = readExpr(ctxt);
			if ( !v.type()->contains(*e->getType()) ) {
				throw TokenException(top, v.type()->nameStr()+" cannot be assigned with "+e->getType()->nameStr());
			}
			return shared_ptr<Stmt>(new AssignStmt(v, e));
		}
	}
}

shared_ptr<Type> Parser::readType() {
	string top = in.peek().text();
	shared_ptr<Type> retType;

	if ( in.canMatch("[") ) {
		shared_ptr<Type> t = readType();
		in.match("]");
		retType = shared_ptr<Type>( new ListType(t) );
	}
	else if ( in.canMatch("{") ) {
		// TODO can record be empty?
		map<string, Var> vars;

		if (!in.canMatch("}")) {
			shared_ptr<Type> t = readType();
			string nm = in.pop().text();
			vars.insert( map<string, Var>::value_type(nm, Var(t, nm)) );
		}
		while (!in.canMatch("}")) {
			in.match(",");
			shared_ptr<Type> t = readType();
			string nm = in.pop().text();
			vars.insert( map<string, Var>::value_type(nm, Var(t, nm)) );
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

	// union types
	if (in.canMatch("|")) {
		retType = shared_ptr<Type>( new UnionType(retType, readType()) );
	}

	return retType;
}

} /* namespace std */
