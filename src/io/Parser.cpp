#include <iostream>
#include <stdexcept>

#include "Parser.h"

namespace std {

Parser::Parser(Lexer &lexer) :
		in(lexer) {

	// initialise atomic types
	vector<shared_ptr<Type>> initial_types {
		shared_ptr<Type>(new AtomicType<bool>("bool")),
		shared_ptr<Type>(new AtomicType<int>("int")),
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
			Token top = in.peek();
			if (top.text() == "type") {
				in.pop();
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

	if (in.canMatch("[")) {
		cout << "read list" << endl;
		vector<shared_ptr<Expr>> list;
		list.push_back( readExpr(ctxt) );
		while (in.canMatch(",")) {
			list.push_back( readExpr(ctxt) );
		}
		in.match("]");
		e = shared_ptr<Expr>(new ListExpr(list));
	}
	else if (ctxt.isVar(t)) {
		in.pop();
		if (in.canMatch("[")) {
			shared_ptr<Expr> index = readExpr(ctxt);

			Var v = ctxt.copyVar(t);

			cout << "list type " << v.type()->isList() << endl;

			e = shared_ptr<Expr>(new ListLookupExpr(ctxt.copyVar(t), index));
			in.match("]");
		}
		else {
			e = shared_ptr<Expr>(new VariableExpr(ctxt.copyVar(t)));
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
				new TypedValue<bool>( (*i).second, true )); // TODO template value tpyes or map string to class
		e = shared_ptr<Expr>(new ConstExpr(v));
	}
	else if (t.text() == "false") {
		in.pop();
		map<string, shared_ptr<Type>>::iterator i = dectypes.find("bool");

		shared_ptr<Value> v = shared_ptr<Value>(
				new TypedValue<bool>( (*i).second, false ));
		e = shared_ptr<Expr>(new ConstExpr(v));
	}
	else if (isdigit(t.text()[0])) {
		int int_value = atoi(in.pop().text().c_str());
		map<string, shared_ptr<Type>>::iterator i = dectypes.find("int");

		shared_ptr<Value> v = shared_ptr<Value>(
				new TypedValue<int>( (*i).second, int_value));
		e = shared_ptr<Expr>(new ConstExpr(v));
	}
	else if (t.text() == "\"") {
		in.pop();
		map<string, shared_ptr<Type>>::iterator i = dectypes.find("string");

		shared_ptr<Value> v = shared_ptr<Value>(
				new TypedValue<string>( (*i).second, in.pop().text()));
		e = shared_ptr<Expr>(new ConstExpr(v));

		in.match("\"");
	}
	else if (in.canMatch("!")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new NotExpr(second));
	}

	// continuation of expression
	if (in.canMatch("+")) {
		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<int, AddOp<int>>(e, second));
	}
	if (in.canMatch("&")) {
		in.match("&"); // second &

		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, AndOp>(e, second));
	}
	else if (e->getType()->nameStr() == "bool" && in.canMatch("=") ) {
		in.match("="); // second =

		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, EquivOp<bool>>(e, second));
	}
	else if ( e->getType()->nameStr() == "int" && in.canMatch("=") ) {
		in.match("="); // second =

		shared_ptr<Expr> second = readExpr(ctxt);
		e = shared_ptr<Expr>(new OpExpr<bool, EquivOp<int>>(e, second));
	}
	else if ( e->getType()->nameStr() == "int" && in.canMatch(">")) {
		if (in.canMatch("=")) {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, GreaterEqualOp<int>>(e, second));
		}
		else {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, GreaterOp<int>>(e, second));
		}
	}
	else if ( e->getType()->nameStr() == "int" && in.canMatch("<")) {
		if (in.canMatch("=")) {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, LessEqualOp<int>>(e, second));
		}
		else {
			shared_ptr<Expr> second = readExpr(ctxt);
			e = shared_ptr<Expr>(new OpExpr<bool, LessOp<int>>(e, second));
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

	cout << "read " << top.text() << endl;

	if (in.canMatch("{")) {
		vector<shared_ptr<Stmt>> stmts;
		while (!in.canMatch("}")) {
			stmts.push_back( readStmt(ctxt) );
		}
		cout << "read }" << endl;
		return shared_ptr<Stmt>(new BlockStmt( stmts )); // no ; after
	}
	// initial assignment
	else if ( top.text() == "bool" || top.text() == "int" || top.text() == "[" ) {
		shared_ptr<Type> type = readType();
		Token name = in.pop();
		ctxt.initialise(type, name);
		Var v = ctxt.copyVar(name);
		if (in.canMatch("=")) {
			shared_ptr<Expr> e = readExpr(ctxt);
			stmt = shared_ptr<Stmt>(new InitStmt(v, e));
		} else {
			stmt = shared_ptr<Stmt>(new InitStmt(v));
		}
	}
	// assignment
	else if ( ctxt.isVar(top) ) {
		Var v = ctxt.copyVar(top);
		in.pop();
		if (in.canMatch("[")) {
			shared_ptr<Expr> index = readExpr(ctxt);
			in.match("]");
			in.match("=");
			shared_ptr<Expr> e = readExpr(ctxt);
			stmt = shared_ptr<Stmt>(new ListAssignStmt(v, e, index));
		}
		else {
			in.match("=");
			shared_ptr<Expr> e = readExpr(ctxt);
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
			cout << "read else" << endl;
			alt = readStmt(ctxt);
		}
		return shared_ptr<Stmt>(new IfStmt(e, list, alt));	// mo ;
	} else if (in.canMatch("return")) {
		stmt = shared_ptr<Stmt>(new ReturnStmt( readExpr(ctxt) ));
	} else if (in.canMatch("print")) {
		stmt = shared_ptr<Stmt>(new PrintStmt(readExpr(ctxt)));
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
		vector<Var> vars;
		vars.push_back(Var(readType(), in.pop().text()));
		while (in.canMatch(",")) {
			vars.push_back(Var(readType(), in.pop().text()));
		}
		in.match("}");
		return shared_ptr<Type>( new RecordType(vars) );
	}
	else if (dectypes.count(top) > 0) {
		in.pop();
		map<string, shared_ptr<Type>>::iterator i = dectypes.find(top);
		return (*i).second;
	}
	else {
		throw runtime_error("could not identify a type");
	}
}

} /* namespace std */
