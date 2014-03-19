#include <iostream>
#include <stdexcept>

#include "Parser.h"

namespace std {

Parser::Parser(Lexer &lexer) :
		in(lexer) {

	// TODO template value tpyes or map string to class
	// make type list static? lookup by sting

	// initialise atomic types
	vector<shared_ptr<Type>> initial_types {
		shared_ptr<Type>(new AtomicType<void>("void")),
		shared_ptr<Type>(new AtomicType<bool>("bool")),
		shared_ptr<Type>(new AtomicType<char>("char")),
		shared_ptr<Type>(new AtomicType<int>("int")),
		shared_ptr<Type>(new AtomicType<double>("real")),
		shared_ptr<Type>(new AtomicType<string>("string"))
	};

	for (shared_ptr<Type> p: initial_types) {
		dectypes.insert( map<string, shared_ptr<Type>>::value_type(p->nameStr(), p) );
	}

}

Parser::~Parser() {
}

Program Parser::read() {
	try {
		while ( !in.empty() ) {
			if (in.canMatch("type")) {
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
	}

	Program p(functions);
	return p;
}

shared_ptr<Expr> Parser::readExpr(ParserContext &ctxt) {
	Token t = in.peek();
	shared_ptr<Expr> e;

	/*
	 * casting
	 */
	if (in.canMatch("(")) {
		shared_ptr<Type> type = readType();
		in.match(")");
		shared_ptr<Expr> inner = readExpr(ctxt);

		if (type->nameStr() == "real" && inner->getType()->nameStr() == "int")
			e = shared_ptr<Expr>(new CastExpr<double, int>(type, inner));
		else if (type->nameStr() == "int" && inner->getType()->nameStr() == "real")
			e = shared_ptr<Expr>(new CastExpr<int, double>(type, inner));
		else {
			throw runtime_error(type->nameStr()+" to "+inner->getType()->nameStr()+" casting not supported");
		}
	}
	else if (in.canMatch("[")) {
		vector<shared_ptr<Expr>> list;
		list.push_back( readExpr(ctxt) );
		while (in.canMatch(",")) {
			list.push_back( readExpr(ctxt) );
		}
		in.match("]");
		e = shared_ptr<Expr>(new ListExpr(list));
	}
	else if (in.canMatch("|")) {
		Token var = in.pop();
		Var v = ctxt.copyVar( var );
		in.match("|");
		if (v.type()->nameStr() == "string") {
			e = shared_ptr<Expr>(new StringLengthExpr(v));
		}
		else {
			e = shared_ptr<Expr>(new ListLengthExpr(v));
		}
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
		}
		while (!in.canMatch("}")) {
			in.match(",");
			string name = in.pop().text();
			in.match(":");
			shared_ptr<Expr> elem = readExpr(ctxt);
			rec_type.insert( map<string, Var>::value_type(name, Var(elem->getType(), name)) );
			vars.insert( map<string, shared_ptr<Expr>>::value_type(name, elem) );
		}

		shared_ptr<Type> type = shared_ptr<Type>( new RecordType(rec_type) );
		e = shared_ptr<Expr>( new RecordExpr(type, vars) );
	}
	else if (ctxt.isVar(t)) {
		Var v = ctxt.copyVar(t);
		in.pop();
		if (v.type()->isList() && in.canMatch("[")) {
			ListType &lt = (ListType &)*v.type();
			shared_ptr<Type> inner_type = lt.innerType();

			shared_ptr<Expr> index = readExpr(ctxt);
			e = shared_ptr<Expr>(new ListLookupExpr(v, inner_type, index));
			in.match("]");
		}
		else if (v.type()->nameStr() == "string" && in.canMatch("[")) {
			shared_ptr<Type> inner_type = (*dectypes.find("char")).second;

			shared_ptr<Expr> index = readExpr(ctxt);
			e = shared_ptr<Expr>(new StringLookupExpr(v, inner_type, index));
			in.match("]");
		}
		else if (in.canMatch("[")) {
			throw runtime_error("cannot get elements of non list types");
		}
		else {
			if (in.canMatch(".")) {	// record member
				if (!v.type()->isRecord()) {
					throw runtime_error("cannot get members of non record types");
				}

				string memb = in.pop().text();
				RecordType &rt = (RecordType &)*v.type();
				shared_ptr<Type> inner_type = rt.memberType(memb).type();
				e = shared_ptr<Expr>(new RecordMemberExpr(v, inner_type, memb));
			}
			else {
				e = shared_ptr<Expr>(new VariableExpr(v));
			}
		}
	}
	else if (functions.count( t.text() ) == 1) {
		in.pop(); // name
		in.match("(");

		FuncMap::iterator i = functions.find( t.text() );
		vector<shared_ptr<Expr>> args;
		if ( !in.canMatch(")") ) {
			args.push_back(readExpr(ctxt));
			while (in.canMatch(",")) {
				args.push_back(readExpr(ctxt));
			}
			in.match(")");
		}
		e = shared_ptr<Expr>( new FuncCallExpr(shared_ptr<Func>(new Func((*i).second)), args) );
	}
	else if (t.text() == "true") {
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

	/*
	 * numrical values
	 */
	else if ( isdigit(t.text()[0]) || t.text()[0] == '-' ) {
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
		map<string, shared_ptr<Type>>::iterator i = dectypes.find("string");

		shared_ptr<Value> v = shared_ptr<Value>(
				new TypedValue<string>( (*i).second, in.pop().text()));
		e = shared_ptr<Expr>(new ConstExpr(v));
		in.match("\"");
	}
	else if (in.canMatch("'")) {
		map<string, shared_ptr<Type>>::iterator i = dectypes.find("char");

		shared_ptr<Value> v = shared_ptr<Value>(
				new TypedValue<string>( (*i).second, in.pop().text()));
		e = shared_ptr<Expr>(new ConstExpr(v));
		in.match("'");
	}
	else if (in.canMatch("!")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new NotExpr(second));
	}
	else {
		throw runtime_error("could not identify expression "+t.text());
	}

	// -----------------------------------
	// optional continuation of expression
	// -----------------------------------

	// first expr is bool
	if (e->getType()->nameStr() == "bool" && in.canMatch("&")) {
		in.match("&"); // second &

		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, bool, AndOp>(e, second));
	} else if (e->getType()->nameStr() == "bool" && in.canMatch("=")) {
		in.match("="); // second =

		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, bool, EquivOp<bool>>(e, second));
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
		e = shared_ptr<Expr>(new OpExpr<bool, char, EquivOp<char>>(e, second));
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
	else if ( e->getType()->nameStr() == "int" && in.canMatch("=") ) {
		in.match("="); // second =
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, int, EquivOp<int>>(e, second));
	}
	else if ( e->getType()->nameStr() == "int" && in.canMatch(">")) {
		if (in.canMatch("=")) {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, int, GreaterEqualOp<int>>(e, second));
		}
		else {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, int, GreaterOp<int>>(e, second));
		}
	}
	else if ( e->getType()->nameStr() == "int" && in.canMatch("<")) {
		if (in.canMatch("=")) {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, int, LessEqualOp<int>>(e, second));
		}
		else {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, int, LessOp<int>>(e, second));
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
		e = shared_ptr<Expr>(new OpExpr<bool, double, EquivOp<double>>(e, second));
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
		ctxt.initialise(type, name.text());
		args.push_back(ctxt.copyVar(name));

		while (in.canMatch(",")) {
			shared_ptr<Type> type = readType();
			Token name = in.pop();
			ctxt.initialise(type, name.text());
			args.push_back(ctxt.copyVar(name));
		}
		in.match(")");
	}

	Func f( name.text(), t, args, readStmt(ctxt) );
	return f;
}

shared_ptr<Stmt> Parser::readStmt(ParserContext &ctxt) {
	shared_ptr<Stmt> stmt;
	Token top = in.peek();

	if (in.canMatch("{")) {
		vector<shared_ptr<Stmt>> stmts;
		while (!in.canMatch("}")) {
			stmts.push_back( readStmt(ctxt) );
		}
		return shared_ptr<Stmt>(new BlockStmt( stmts )); // no ; after
	}
	// initial assignment
	else if ( dectypes.count( top.text() ) > 0 || top.text() == "[" ) {	// list types begin with [
		shared_ptr<Type> type = readType();
		Token name = in.pop();
		ctxt.initialise(type, name);
		Var v = ctxt.copyVar(name);
		if (in.canMatch("=")) { // ok to skip initial assignment
			shared_ptr<Expr> e = readExpr(ctxt);
			if (!((*type) == (*e->getType()))) {
				throw runtime_error(type->nameStr() + " type doesnt match "+ e->getType()->nameStr());
			}
			stmt = shared_ptr<Stmt>(new InitStmt(v, e));
		}
		else {
			stmt = shared_ptr<Stmt>(new InitStmt(v));
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
			if (!( (*inner_type) == (*e->getType()) )) {
				throw runtime_error(v.type()->nameStr()+" doesnt match "+e->getType()->nameStr());
			}
			stmt = shared_ptr<Stmt>(new ListAssignStmt(v, e, index));
		}
		if (in.canMatch(".")) {
			string member = in.pop().text();
			RecordType &rt = (RecordType &)*v.type();
			shared_ptr<Type> member_type = rt.memberType(member).type();

			in.match("=");
			shared_ptr<Expr> e = readExpr(ctxt);
			if (!( (*member_type) == (*e->getType()) )) {
				throw runtime_error(member_type->nameStr()+" doesnt match "+e->getType()->nameStr());
			}
			stmt = shared_ptr<Stmt>(new RecordAssignStmt(v, e, member));
		}
		else {
			in.match("=");
			shared_ptr<Expr> e = readExpr(ctxt);
			if (!( (*v.type()) == (*e->getType()) )) {
				throw runtime_error(v.type()->nameStr()+" doesnt match "+e->getType()->nameStr());
			}
			stmt = shared_ptr<Stmt>(new AssignStmt(v, e));
		}
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
		return shared_ptr<Stmt>(new IfStmt(e, list, alt));	// mo ;
	}
	else if (in.canMatch("while")) {
		in.match("(");
		shared_ptr<Expr> e = readExpr(ctxt);
		in.match(")");
		shared_ptr<Stmt> list = readStmt(ctxt);
		return shared_ptr<Stmt>(new WhileStmt(e, list));	// mo ;
	}
	else if (in.canMatch("return")) {
		stmt = shared_ptr<Stmt>(new ReturnStmt( readExpr(ctxt) ));
	} else if (in.canMatch("print")) {
		stmt = shared_ptr<Stmt>(new PrintStmt(readExpr(ctxt)));
	}
	else {
		throw runtime_error("could not identify a statement from "+top.text());
	}
	in.match(";");
	return stmt;
}

shared_ptr<Type> Parser::readType() {
	string top = in.peek().text();

	if ( in.canMatch("[") ) {
		shared_ptr<Type> t = readType();
		in.match("]");
		return shared_ptr<Type>( new ListType(t) );
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
		return shared_ptr<Type>( new RecordType(vars) );
	}
	else if (dectypes.count(top) > 0) {
		in.pop();
		map<string, shared_ptr<Type>>::iterator i = dectypes.find(top);
		return (*i).second;
	}
	else {
		throw runtime_error("could not identify a type from "+top);
	}
}

} /* namespace std */
