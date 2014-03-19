/*
 * Expr.h
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#ifndef EXPR_H_
#define EXPR_H_

#include <iostream>
#include <string>

#include "Common.h"
#include "Type.h"
#include "SyntaxElem.h"

namespace std {

class Func;

class Expr: public SyntaxElem {
public:
	Expr(shared_ptr<Type> t): type(t) {}

	shared_ptr<Type> getType() {
		return type;
	}

	virtual shared_ptr<Value> eval( Stack &, VarMap m ) = 0;

private:
	shared_ptr<Type> type;

};

class ConstExpr: public Expr {
public:
	ConstExpr(shared_ptr<Value> c): Expr( c->type() ) {
		const_value = c;
	}

	shared_ptr<Value> eval( Stack &, VarMap m ) {
		return const_value;
	}

private:
	shared_ptr<Value> const_value;

};

class VariableExpr: public Expr {
public:
	VariableExpr(Var v): Expr( v.type() ), var(v) {}

	shared_ptr<Value> eval( Stack &, VarMap m ) {
		if ( m.count(var) == 0 ) { //
			throw runtime_error("error evaluating variable "+var.name()+": not available");
		}
		return m[var];
	}

private:
	Var var;
};

class ListExpr: public Expr {
public:
	ListExpr( vector<shared_ptr<Expr>> l ): Expr( shared_ptr<Type>( new ListType( l[0]->getType() ) ) ) {
		list = l;
	}

	shared_ptr<Value> eval( Stack &s, VarMap m ) {
		// evaluate each item, return list of values
		ValueList eval_list;
		for ( shared_ptr<Expr> ex: list ) {
			eval_list.push_back( ex->eval( s, m ));
		}

		return shared_ptr<Value>( new TypedValue<ValueList>( getType(), eval_list ) );
	}

private:
	vector<shared_ptr<Expr>> list;
};

class ListLookupExpr: public Expr {
public:
	ListLookupExpr( Var v, shared_ptr<Expr> i ): Expr( v.type() ), var(v) { // TODO fix types, should be internal
		index = i;
	}

	shared_ptr<Value> eval( Stack &s, VarMap m ) {
		if ( m.count(var) == 0 ) {
			throw runtime_error("error evaluating variable "+var.name()+": not available");
		}
		if ( !m[var]->type()->isList() ) {
			throw runtime_error("error evaluating list "+var.name()+": not a list");
		}

		shared_ptr<TypedValue<ValueList>> list = static_pointer_cast<TypedValue<ValueList>, Value>( m[var] );
		shared_ptr<TypedValue<int>> i = static_pointer_cast<TypedValue<int>, Value>( index->eval( s, m ) );

		if ( i->value() < 0 || list->value().size() <= i->value() ) {
			throw runtime_error("error evaluating list "+var.name()+": index out of range");
		}

		shared_ptr<Value> result = list->value()[ i->value() ];

		return result;
	}

private:
	Var var;
	shared_ptr<Expr> index;
};

class FuncCallExpr: public Expr {
public:
	FuncCallExpr( shared_ptr<Func> f, vector<shared_ptr<Expr>> a );

	shared_ptr<Value> eval( Stack &, VarMap m );

private:
	shared_ptr<Func> func;
	vector<shared_ptr<Expr>> args;
};

template<class T> struct AddOp {
	static T compute(T a, T b) {
		return a + b;
	}
};

struct AndOp {
	static bool compute(bool a, bool b) {
		return a == b;
	}
};

template<class T> struct EquivOp {
	static T compute(T a, T b) {
		return a == b;
	}
};

template<class T> struct GreaterOp {
	static T compute(T a, T b) {
		return a > b;
	}
};

template<class T> struct GreaterEqualOp {
	static T compute(T a, T b) {
		return a >= b;
	}
};

template<class T> struct LessOp {
	static T compute(T a, T b) {
		return a < b;
	}
};

template<class T> struct LessEqualOp {
	static T compute(T a, T b) {
		return a <= b;
	}
};

template<class T, class O> class OpExpr: public Expr {
public:
	OpExpr(shared_ptr<Expr> a, shared_ptr<Expr> b): Expr( a->getType() ) {
		first = a;
		second = b;
	}

	shared_ptr<Value> eval( Stack &s, VarMap m ) {
		shared_ptr<TypedValue<T>> a = static_pointer_cast<TypedValue<T>, Value>( first->eval( s, m ) );
		shared_ptr<TypedValue<T>> b = static_pointer_cast<TypedValue<T>, Value>( second->eval( s, m ) );

		T result = O::compute(a->value(),  b->value());
		return shared_ptr<Value>( new TypedValue<T>( first->getType(), result ) );
	}

private:
	shared_ptr<Expr> first;
	shared_ptr<Expr> second;
};

class NotExpr: public Expr {
public:
	NotExpr(shared_ptr<Expr> a): Expr( a->getType() ) {
		first = a;
	}

	shared_ptr<Value> eval( Stack &s, VarMap m ) {
		shared_ptr<TypedValue<bool>> a = static_pointer_cast<TypedValue<bool>, Value>( first->eval( s, m ) );

		bool result = !a->value();
		return shared_ptr<Value>( new TypedValue<bool>( first->getType(), result ) );
	}

private:
	shared_ptr<Expr> first;
};

} /* namespace std */

#endif /* EXPR_H_ */
