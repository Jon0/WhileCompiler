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



class RecordExpr: public Expr {
public:
	RecordExpr( shared_ptr<Type> t, map<string, shared_ptr<Expr>> r ): Expr(t) {
		rec = r;
	}

	shared_ptr<Value> eval( Stack &s, VarMap m ) {
		// evaluate each item, return list of values
		ValueRecord eval_list;
		for ( map<string, shared_ptr<Expr>>::value_type ex: rec ) {
			eval_list.insert( ValueRecord::value_type(ex.first, ex.second->eval( s, m ) ) );
		}

		return shared_ptr<Value>( new TypedValue<ValueRecord>( getType(), eval_list ) );
	}

private:
	map<string, shared_ptr<Expr>> rec;
};

class ListExpr: public Expr {
public:
	// TODO generate list type if int and bool are contained, type is bool|int
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

class ListLengthExpr: public Expr {
public:
	// TODO generate list type if int and bool are contained, type is bool|int
	ListLengthExpr( Var v ): Expr( shared_ptr<Type>( new AtomicType<int>("int") ) ) {
		var = v;
	}

	shared_ptr<Value> eval( Stack &s, VarMap m ) {
		if ( m.count(var) == 0 ) {
			throw runtime_error("error evaluating variable "+var.name()+": not available");
		}
		if ( !m[var]->type()->isList() ) {
			throw runtime_error("error evaluating list "+var.name()+": not a list");
		}

		shared_ptr<TypedValue<ValueList>> list = static_pointer_cast<TypedValue<ValueList>, Value>( m[var] );
		return shared_ptr<Value>( new TypedValue<int>( getType(), list->value().size() ) );
	}

private:
	Var var;
};

/*
 * Var v is the List variable
 * t is the type of inner elements(what is returned from this)
 * i is the position in the list to lookup
 */
class ListLookupExpr: public Expr {
public:
	ListLookupExpr( Var v, shared_ptr<Type> t, shared_ptr<Expr> i ): Expr( t ), var(v) {
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

class StringLookupExpr: public Expr {
public:
	StringLookupExpr( Var v, shared_ptr<Type> t, shared_ptr<Expr> i ): Expr( t ), var(v) {
		index = i;
	}

	shared_ptr<Value> eval( Stack &s, VarMap m ) {
		if ( m.count(var) == 0 ) {
			throw runtime_error("error evaluating variable "+var.name()+": not available");
		}

		shared_ptr<TypedValue<string>> list = static_pointer_cast<TypedValue<string>, Value>( m[var] );
		shared_ptr<TypedValue<int>> i = static_pointer_cast<TypedValue<int>, Value>( index->eval( s, m ) );

		if ( i->value() < 0 || list->value().length() <= i->value() ) {
			throw runtime_error("error evaluating list "+var.name()+": index out of range ("+to_string(i->value())+", "+to_string(list->value().length())+")");
		}
		return shared_ptr<Value>( new TypedValue<char>( getType(), list->value()[i->value()] ));
	}

private:
	Var var;
	shared_ptr<Expr> index;
};

class StringLengthExpr: public Expr {
public:
	// TODO generate list type if int and bool are contained, type is bool|int
	StringLengthExpr( Var v ): Expr( shared_ptr<Type>( new AtomicType<int>("int") ) ) {
		var = v;
	}

	shared_ptr<Value> eval( Stack &s, VarMap m ) {
		if ( m.count(var) == 0 ) {
			throw runtime_error("error evaluating variable "+var.name()+": not available");
		}

		shared_ptr<TypedValue<string>> list = static_pointer_cast<TypedValue<string>, Value>( m[var] );
		return shared_ptr<Value>( new TypedValue<int>( getType(), list->value().length() ) );
	}

private:
	Var var;
};

/*
 * Var v is the Record variable
 * t is the type of inner elements(what is returned from this)
 * m_name is the name of the member to get
 */
class RecordMemberExpr: public Expr {
public:
	RecordMemberExpr( Var v, shared_ptr<Type> t, string m_name ): Expr( t ), var(v) {
		member_name = m_name;
	}

	shared_ptr<Value> eval( Stack &s, VarMap m ) {
		if ( m.count(var) == 0 ) {
			throw runtime_error("error evaluating variable "+var.name()+": not available");
		}
		if ( !m[var]->type()->isRecord() ) {
			throw runtime_error("error evaluating record "+var.name()+": not a record");
		}

		shared_ptr<TypedValue<ValueRecord>> record = static_pointer_cast<TypedValue<ValueRecord>, Value>( m[var] );

		if ( record->value().count(member_name) == 0 ) {
			throw runtime_error("error evaluating record: "+member_name+" not defined");
		}
		shared_ptr<Value> result = record->value()[ member_name ];
		return result;
	}

private:
	Var var;
	string member_name;
};

class FuncCallExpr: public Expr {
public:
	FuncCallExpr( shared_ptr<Func> f, vector<shared_ptr<Expr>> a );

	shared_ptr<Value> eval( Stack &, VarMap m );

private:
	shared_ptr<Func> func;
	vector<shared_ptr<Expr>> args;
};

template<class R, class T> class CastExpr: public Expr {
public:
	CastExpr( shared_ptr<Type> t, shared_ptr<Expr> e ): Expr( t ) {
		expr = e;
	}

	shared_ptr<Value> eval( Stack &s, VarMap m ) {
		if (getType()->isList() || getType()->isRecord()) {
			throw runtime_error("list or record cast not yet supported");
		}
		shared_ptr<Value> v = expr->eval( s, m );

		shared_ptr<TypedValue<T>> i = static_pointer_cast<TypedValue<T>, Value>( v );

		return shared_ptr<Value>( new TypedValue<R>(getType(), (R)i->value() ) );
	}

private:
	shared_ptr<Expr> expr;
};


template<class T> struct AddOp {
	static T compute(T a, T b) {
		return a + b;
	}
};

template<class T> struct SubOp {
	static T compute(T a, T b) {
		return a - b;
	}
};

template<class T> struct MulOp {
	static T compute(T a, T b) {
		return a * b;
	}
};

template<class T> struct DivOp {
	static T compute(T a, T b) {
		return a / b;
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

/*
 * R is result type
 * T is type of values
 * O defines and operation
 * eg. 1 == 1 uses <bool, int, EquivOp>
 */
template<class R, class T, class O> class OpExpr: public Expr {
public:
	OpExpr(shared_ptr<Expr> a, shared_ptr<Expr> b): Expr( a->getType() ) {
		first = a;
		second = b;
	}

	shared_ptr<Value> eval( Stack &s, VarMap m ) {
		shared_ptr<TypedValue<T>> a = static_pointer_cast<TypedValue<T>, Value>( first->eval( s, m ) );
		shared_ptr<TypedValue<T>> b = static_pointer_cast<TypedValue<T>, Value>( second->eval( s, m ) );

		R result = O::compute(a->value(),  b->value());
		return shared_ptr<Value>( new TypedValue<R>( first->getType(), result ) );
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
