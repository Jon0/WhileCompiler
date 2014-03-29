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

#include "Type.h"
#include "SyntaxElem.h"
#include "Value.h"
#include "Var.h"

namespace std {

class Func;

class Expr: public SyntaxElem {
public:
	Expr(shared_ptr<Type> ty): type(ty) {}

	Expr(Token tok, shared_ptr<Type> ty): type(ty) {
		addToken(tok);
	}

	shared_ptr<Type> getType() {
		return type;
	}

	virtual shared_ptr<Value> eval( Stack &, VarMap &m, shared_ptr<Value> **p ) = 0;

	shared_ptr<Value> eval( Stack &s, VarMap &m ) {
		return eval( s, m, NULL );
	}

	virtual void typeCheck( CheckState & ) = 0;

	virtual shared_ptr<Var>  assignable( CheckState & ) {
		return NULL;
	}

private:
	shared_ptr<Type> type;

};

class ConstExpr: public Expr {
public:
	ConstExpr(shared_ptr<Value> c): Expr( c->type() ) {
		const_value = c;
	}

	ConstExpr(Token tok, shared_ptr<Value> c): Expr( tok, c->type() ) {
		const_value = c;
	}

	shared_ptr<Value> eval( Stack &, VarMap &m, shared_ptr<Value> **p ) {
		return const_value;
	}

	void typeCheck( CheckState & ) {}

private:
	shared_ptr<Value> const_value;

};

/*
 * check types
 * TODO: very broken...
 */
class IsTypeExpr: public Expr {
public:
	IsTypeExpr( Token tok, shared_ptr<Expr> e, shared_ptr<Type> t): Expr( tok, shared_ptr<Type>( new AtomicType<bool>("bool") )  ) {
		to_check = e;
		type = t;
	}

	shared_ptr<Value> eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {
		shared_ptr<Type> subt = to_check->eval(s, m)->type();
		return shared_ptr<Value>( new TypedValue<bool>( getType(), type->contains(*subt) ) );
	}

	void typeCheck( CheckState & ) {
		throw runtime_error("is expr type check not implemented");
	}

private:
	shared_ptr<Expr> to_check;
	shared_ptr<Type> type;

};

class VariableExpr: public Expr {
public:
	VariableExpr(Token tok, shared_ptr<Var> v): Expr( tok, v->type() ) {
		var = v;
	}

	shared_ptr<Value> eval( Stack &, VarMap &m, shared_ptr<Value> **p ) {
		if ( m.count(*var) == 0 ) { //
			throw runtime_error("error evaluating variable "+var->name()+": not available");
		}
		if (p) {
			*p = &m[*var];
		}
		return m[*var];
	}

	void typeCheck( CheckState &cs ) {
		map<string, AssignState>::iterator i = cs.assigned.find( var->name() );
		if ( i == cs.assigned.end() || !(*i).second.defAssign ) {
			throw TokenException(getToken(0), "variable "+var->name()+" is not definitely assigned");
		}
	}

	shared_ptr<Var> assignable( CheckState & ) {
		return var;
	}

private:
	shared_ptr<Var> var;
};

class RecordExpr: public Expr {
public:
	RecordExpr( Token tok, shared_ptr<Type> t, map<string, shared_ptr<Expr>> r ): Expr(tok, t) {
		rec = r;
	}

	shared_ptr<Value> eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {
		// evaluate each item, return list of values
		ValueRecord eval_list;
		for ( map<string, shared_ptr<Expr>>::value_type ex: rec ) {
			eval_list.insert( ValueRecord::value_type(ex.first, ex.second->eval( s, m ) ) );
		}

		return shared_ptr<Value>( new TypedValue<ValueRecord>( getType(), eval_list ) );
	}

	void typeCheck( CheckState &cs ) {
		for ( map<string, shared_ptr<Expr>>::value_type ex: rec ) {
			ex.second->typeCheck(cs);
		}
	}

private:
	map<string, shared_ptr<Expr>> rec;
};

class ListExpr: public Expr {
public:
	// TODO generate list type if int and bool are contained, type is bool|int
	ListExpr( Token tok, shared_ptr<Type> t, vector<shared_ptr<Expr>> l ): Expr( tok, t ) {
		list = l;
	}

	shared_ptr<Value> eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {
		// evaluate each item, return list of values
		ValueList eval_list;
		for ( shared_ptr<Expr> ex: list ) {
			eval_list.push_back( ex->eval( s, m ));
		}

		return shared_ptr<Value>( new TypedValue<ValueList>( getType(), eval_list ) );
	}

	void typeCheck( CheckState &cs ) {
		for ( shared_ptr<Expr> ex: list ) {
			ex->typeCheck(cs);
		}
	}

private:
	vector<shared_ptr<Expr>> list;
};

class ListLengthExpr: public Expr {
public:
	ListLengthExpr( Token tok, shared_ptr<Expr> v ): Expr( tok, shared_ptr<Type>( new AtomicType<int>("int") ) ) {
		e = v;
	}

	shared_ptr<Value> eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {
		shared_ptr<Value> vl = e->eval(s, m);

		if ( !vl->type()->isList() ) {
			throw runtime_error("error evaluating list length: not a list");
		}

		shared_ptr<Value> rs = e->eval(s, m);
		shared_ptr<TypedValue<ValueList>> list = static_pointer_cast<TypedValue<ValueList>, Value>( rs );
		return shared_ptr<Value>( new TypedValue<int>( getType(), list->value().size() ) );
	}

	void typeCheck( CheckState & ) {
		if ( !e->getType()->isList() ) {
			throw TokenException(getToken(0), "only accepts list types");
		}
	}

private:
	shared_ptr<Expr> e;
};

class ConcatExpr: public Expr {
public:
	ConcatExpr( Token tok, shared_ptr<Type> t, shared_ptr<Expr> a, shared_ptr<Expr> b  ): Expr( tok, t ) { // result type is list
		first = a;	// TODO check first is a list type
		second = b;
	}

	shared_ptr<Value> eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {
		shared_ptr<TypedValue<ValueList>> list1 = static_pointer_cast<TypedValue<ValueList>, Value>( first->eval(s,m)->clone() );
		ValueList newList = list1->value();

		/*
		 * concat with single value or list?
		 */
		if ( !second->getType()->isList() ) {
			newList.push_back( second->eval(s,m) );
		}
		else {
			// hack to make strings work
			if (getType()->nameStr() == "string") {
				string insert = second->eval(s,m)->asString();
				shared_ptr<Type> inner_type = shared_ptr<Type>(new AtomicType<char>("char"));
				for (char c: insert) {
					newList.push_back( makeValue<char>(inner_type, c) );
				}
			}
			else {
				shared_ptr<TypedValue<ValueList>> list2 = static_pointer_cast<TypedValue<ValueList>, Value>( second->eval(s,m)->clone() );
				newList.insert(newList.end(), list2->value().begin(), list2->value().end());
			}
		}

		return shared_ptr<Value>( new TypedValue<ValueList>( getType(), newList) );
	}

	void typeCheck( CheckState &cs ) {
		if (!first->getType()->isList()) {
			throw TokenException(getToken(0), "concat used on non list type");
		}
		else {
			shared_ptr<ListType> list_t = static_pointer_cast<ListType, Type>(
					first->getType());
			if (!(getType()->nameStr() == "string" || list_t->innerType()->contains(*second->getType())
					|| list_t->contains(*second->getType()))) {
				throw TokenException(getToken(0),
						"operands must have identical types, found "
								+ first->getType()->nameStr() + " and "
								+ second->getType()->nameStr());
			}

		}

		first->typeCheck(cs);
		second->typeCheck(cs);
	}

private:
	shared_ptr<Expr> first;
	shared_ptr<Expr> second;
};

/*
 * Var v is the List variable
 * t is the type of inner elements(what is returned from this)
 * i is the position in the list to lookup
 */
class ListLookupExpr: public Expr {
public:
	ListLookupExpr( Token tok, shared_ptr<Expr> v, shared_ptr<Type> t, shared_ptr<Expr> i ): Expr( tok, t ) {
		v_expr = v;
		index = i;
	}

	shared_ptr<Value> eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {
		if ( !v_expr->getType()->isList() ) {
			throw runtime_error("cannot lookup type "+v_expr->getType()->nameStr()+": not a list");
		}

		shared_ptr<TypedValue<ValueList>> list = static_pointer_cast<TypedValue<ValueList>, Value>( v_expr->eval( s, m ) );
		shared_ptr<TypedValue<int>> i = static_pointer_cast<TypedValue<int>, Value>( index->eval( s, m ) );

		if ( i->value() < 0 || list->value().size() <= i->value() ) {
			throw runtime_error("error evaluating index "+to_string(i->value())+": index out of range");
		}

		if (p) {
			*p = &list->value()[ i->value() ];
		}
		return list->value()[ i->value() ];
	}

	void typeCheck( CheckState &c ) {
		v_expr->typeCheck(c);
		index->typeCheck(c);
		if (index->getType()->nameStr() != "int") {
			throw TokenException(index->getToken(0),
					"expected type int, found " + index->getType()->nameStr());
		}
	}

	shared_ptr<Var> assignable( CheckState &c ) {
		typeCheck(c);
		return v_expr->assignable(c);
	}

private:
	shared_ptr<Expr> v_expr;
	shared_ptr<Expr> index;
};

/*
 * Var v is the Record variable
 * t is the type of inner elements(what is returned from this)
 * m_name is the name of the member to get
 */
class RecordMemberExpr: public Expr {
public:
	RecordMemberExpr( Token tok, shared_ptr<Expr> v, shared_ptr<Type> t, string m_name ): Expr( tok, t ) {
		v_expr = v;
		member_name = m_name;
	}

	shared_ptr<Value> eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {
		if ( !v_expr->getType()->isRecord() ) {
			throw runtime_error("cannot lookup type "+v_expr->getType()->nameStr()+": not a record");
		}

		shared_ptr<TypedValue<ValueRecord>> record = static_pointer_cast<TypedValue<ValueRecord>, Value>( v_expr->eval( s, m ) );

		if ( record->value().count(member_name) == 0 ) {
			throw runtime_error("error evaluating record: "+member_name+" not defined");
		}

		if (p) {
			*p = &record->value()[ member_name ];
		}
		return record->value()[ member_name ];
	}

	void typeCheck( CheckState &c ) {
		v_expr->typeCheck(c);
	}

	shared_ptr<Var> assignable( CheckState &c ) {
		v_expr->typeCheck(c);
		return v_expr->assignable(c);
	}

private:
	shared_ptr<Expr> v_expr;
	string member_name;
};

class FuncCallExpr: public Expr {
public:
	FuncCallExpr( Token tok, shared_ptr<Func> f, vector<shared_ptr<Expr>> a );

	shared_ptr<Value> eval( Stack &, VarMap &m, shared_ptr<Value> **p );

	void typeCheck( CheckState &cs );

private:
	shared_ptr<Func> func;
	vector<shared_ptr<Expr>> args;
};

class BasicCastExpr: public Expr {
public:
	BasicCastExpr( Token tok, shared_ptr<Type> t, shared_ptr<Expr> e ): Expr( tok, t ) {
		expr = e;
	}

	shared_ptr<Value> eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {
		shared_ptr<Value> v = expr->eval( s, m );

		if ( !v->type()->castsTo(*getType()) ) {
			throw runtime_error("invalid casting values: "+v->type()->nameStr()+" to "+getType()->nameStr());
		}

		return v->clone( getType() ) ;
	}

	void typeCheck( CheckState & ) {
		throw runtime_error("basic cast expr type check not implemented");
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

template<class T> struct ModOp {
	static T compute(T a, T b) {
		return (int)a % (int)b;
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
	OpExpr(shared_ptr<Type> t, shared_ptr<Expr> a, shared_ptr<Expr> b): Expr( t ) {
		first = a;
		second = b;
		copyTokens(*first);
		copyTokens(*second);
	}

	OpExpr(shared_ptr<Expr> a, shared_ptr<Expr> b): Expr( a->getType() ) {
		first = a;
		second = b;
	}

	shared_ptr<Value> eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {
		shared_ptr<TypedValue<T>> a = static_pointer_cast<TypedValue<T>, Value>( first->eval( s, m ) );
		shared_ptr<TypedValue<T>> b = static_pointer_cast<TypedValue<T>, Value>( second->eval( s, m ) );
		R result = O::compute(a->value(),  b->value());
		return shared_ptr<Value>( new TypedValue<R>( first->getType(), result ) );
	}

	void typeCheck( CheckState &cs ) {
		first->typeCheck(cs);
		second->typeCheck(cs);
		if (*first->getType() != *second->getType() ) {
			throw TokenException(getToken(0), "operator not compatible");
		}
	}

private:
	shared_ptr<Expr> first;
	shared_ptr<Expr> second;
};

class EquivOp: public Expr {
public:
	EquivOp(Token tok, shared_ptr<Type> t, shared_ptr<Expr> a, shared_ptr<Expr> b): Expr( tok, t ) {
		first = a;
		second = b;
	}

	shared_ptr<Value> eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {
		bool result = *first->eval( s, m ) == *second->eval( s, m );
		return shared_ptr<Value>( new TypedValue<bool>( first->getType(), result ) );
	}

	void typeCheck( CheckState & ) {}

private:
	shared_ptr<Expr> first;
	shared_ptr<Expr> second;
};

class NotEquivOp: public Expr {
public:
	NotEquivOp(Token tok, shared_ptr<Type> t, shared_ptr<Expr> a, shared_ptr<Expr> b): Expr( tok, t ) {
		first = a;
		second = b;
	}

	shared_ptr<Value> eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {
		bool result = *first->eval( s, m ) != *second->eval( s, m );
		return shared_ptr<Value>( new TypedValue<bool>( getType(), result ) );
	}

	void typeCheck( CheckState & ) {}

private:
	shared_ptr<Expr> first;
	shared_ptr<Expr> second;
};

class AndExpr: public Expr {
public:
	AndExpr(Token tok, shared_ptr<Type> t, shared_ptr<Expr> a, shared_ptr<Expr> b): Expr( tok, t ) {
		first = a;
		second = b;
	}

	shared_ptr<Value> eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {
		shared_ptr<TypedValue<bool>> a = static_pointer_cast<TypedValue<bool>, Value>( first->eval( s, m ) );
		bool result;
		if ( !a->value() ) {
			result = false;
		}
		else {
			shared_ptr<TypedValue<bool>> b = static_pointer_cast<TypedValue<bool>, Value>( second->eval( s, m ) );
			if ( !b->value() ) {
				result = false;
			}
			else {
				result = true;
			}
		}
		return shared_ptr<Value>( new TypedValue<bool>( getType(), result ) );
	}

	void typeCheck( CheckState & ) {
		throw runtime_error("and expr type check not implemented");
	}

private:
	shared_ptr<Expr> first;
	shared_ptr<Expr> second;
};

class OrExpr: public Expr {
public:
	OrExpr(Token tok, shared_ptr<Type> t, shared_ptr<Expr> a, shared_ptr<Expr> b): Expr( tok, t ) {
		first = a;
		second = b;
	}

	shared_ptr<Value> eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {
		shared_ptr<TypedValue<bool>> a = static_pointer_cast<TypedValue<bool>, Value>( first->eval( s, m ) );
		bool result;
		if ( a->value() ) {
			result = true;
		}
		else {
			shared_ptr<TypedValue<bool>> b = static_pointer_cast<TypedValue<bool>, Value>( second->eval( s, m ) );
			if ( b->value() ) {
				result = true;
			}
			else {
				result = false;
			}
		}
		return shared_ptr<Value>( new TypedValue<bool>( getType(), result ) );
	}

	void typeCheck( CheckState & ) {
		throw runtime_error("or expr type check not implemented");
	}

private:
	shared_ptr<Expr> first;
	shared_ptr<Expr> second;
};

class NotExpr: public Expr {
public:
	NotExpr(Token tok, shared_ptr<Expr> a): Expr( tok, a->getType() ) {
		first = a;
	}

	shared_ptr<Value> eval( Stack &s, VarMap &m, shared_ptr<Value> **p ) {
		shared_ptr<TypedValue<bool>> a = static_pointer_cast<TypedValue<bool>, Value>( first->eval( s, m ) );

		bool result = !a->value();
		return shared_ptr<Value>( new TypedValue<bool>( getType(), result ) );
	}

	void typeCheck( CheckState &cs ) {
		throw runtime_error("not expr type check not implemented");
	}

private:
	shared_ptr<Expr> first;
};

} /* namespace std */

#endif /* EXPR_H_ */
