/*
 * Type.h
 *
 *  Created on: 16/03/2014
 *      Author: remnanjona
 */

#ifndef TYPE_H_
#define TYPE_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "SyntaxElem.h"
#include "Value.h"

namespace std {

/*
 * in/out
 */
template<class I, class O> class Caster {
public:
	static O func( shared_ptr<Value> in ) { // shared_ptr<Type> type, shared_ptr<Value> v
		shared_ptr<TypedValue<I>> iptr = static_pointer_cast<TypedValue<I>, Value>( in );
		return (O) iptr->value();
	}
};

/*
 * switch function usig the highest level type, calls with templating as required
 */
template<template<class I, class O> class Func, class Return, class Source> class TypeSwitch {
public:
	static Return typeSwitch( shared_ptr<Type> type, Source in ) {

		if (type->nameStr() == "char") {
			return Func<char, Return>::func( in );
		}
		else if (type->nameStr() == "int") {
			return Func<int, Return>::func( in );
		}
		else if (type->nameStr() == "real") {
			return Func<double, Return>::func( in );
		}
		else if (type->nameStr() == "bool") {
			return Func<bool, Return>::func( in );
		}
		else if (type->isList()) {
			//return T<string>::func(type, in);
		}
	}
};

class VoidType: public Type {

	virtual shared_ptr<Value> createValue( shared_ptr<Value> v ) {
		return v;
	}

	virtual bool castsTo( const Type &other ) const {
		return false;
	}

	virtual bool contains( const Type &other ) const {
		return false;
	}

	virtual bool operator==( const Type &other ) const {
		return false;
	}

	virtual bool isNull() const {
		return false;
	}

	virtual bool isAtomic() const {
		return false;
	}

	virtual bool isUnion() const {
		return false;
	}

	virtual bool isList() const {
		return false;
	}

	virtual bool isRecord() const {
		return false;
	}

	virtual string nameStr() const {
		return "void";
	}
};

class NullType: public Type {

	virtual shared_ptr<Value> createValue( shared_ptr<Value> v ) {
		return v;
	}

	virtual bool castsTo( const Type &other ) const {
		return false;
	}

	virtual bool contains( const Type &other ) const {
		return false;
	}

	virtual bool operator==( const Type &other ) const {
		return other.isNull();
	}

	virtual bool isNull() const {
		return true;
	}

	virtual bool isAtomic() const {
		return false;
	}

	virtual bool isUnion() const {
		return false;
	}

	virtual bool isList() const {
		return false;
	}

	virtual bool isRecord() const {
		return false;
	}

	virtual string nameStr() const {
		return "null";
	}
};

template<class T> class AtomicType: public Type {
public:
	typedef T InternalType;

	AtomicType(string n) {
		name = n;
	}

	virtual ~AtomicType() {

	}

	virtual shared_ptr<Value> createValue( shared_ptr<Value> v ) {
		T new_internal = TypeSwitch<Caster, T, shared_ptr<Value>>::typeSwitch( v->type(), v );

		return shared_ptr<Value>( new TypedValue<T>(shared_from_this(), new_internal) );
	}

	virtual bool castsTo( const Type &other ) const {
		return other.isAtomic();
	}

	virtual bool contains( const Type &other ) const {
		return (*this) == other;
	}

	virtual bool operator==( const Type &other ) const {
		if ( other.isAtomic() ) {
			AtomicType &other_at = (AtomicType &)other;
			return name == other_at.name; // TODO check types
		}
		return false;
	}

	virtual bool isNull() const {
		return false;
	}

	virtual bool isAtomic() const {
		return true;
	}

	virtual bool isUnion() const {
		return false;
	}

	virtual bool isList() const {
		return false;
	}

	virtual bool isRecord() const {
		return false;
	}

	virtual string nameStr() const {
		return name;
	}

private:
	string name;
};

class ListType: public Type {
public:
	ListType() {
		elem_type = NULL;
	}

	ListType(shared_ptr<Type> n) {
		elem_type = n;
	}

	virtual shared_ptr<Value> createValue( shared_ptr<Value> in ) {
		shared_ptr<TypedValue<ValueList>> iptr = static_pointer_cast<TypedValue<ValueList>, Value>( in );

		// cast each value in list
		ValueList newList;
		for ( shared_ptr<Value> sp: iptr->value() ) {
			newList.push_back( elem_type->createValue(sp) );
		}

		return make_shared<TypedValue<ValueList>>( shared_from_this(), newList );
	}

	virtual bool castsTo( const Type &other ) const {
		if ( other.isList() ) {
			ListType &other_list = (ListType &)other;
			return elem_type->castsTo(*other_list.elem_type);
		}
		return false;
	}


	virtual bool contains( const Type &other ) const {
		if ( elem_type && other.isList() ) {
			ListType &other_list = (ListType &)other;

			// consider nulls(empty list) a subtype
			return other_list.elem_type == NULL || elem_type->contains(*other_list.elem_type);
		}
		return false;
	}

	virtual bool operator==( const Type &other ) const {
		if ( other.isList() ) {
			ListType &other_list = (ListType &)other;
			if ((elem_type == NULL || other_list.elem_type == NULL) && elem_type != other_list.elem_type) {
				return false;
			}

			// nulls(empty list) should return false, unless both null
			return (elem_type == NULL && other_list.elem_type == NULL) || *elem_type == *other_list.elem_type;
		}
		return false;
	}

	virtual bool isNull() const {
		return false;
	}

	virtual bool isAtomic() const {
		return false;
	}

	virtual bool isUnion() const {
		return false;
	}

	virtual bool isList() const {
		return true;
	}

	virtual bool isRecord() const {
		return false;
	}

	virtual string nameStr() const {
		if (elem_type) {
			return "[" + elem_type->nameStr() + "]";
		}
		else {
			return "[]";
		}
	}

	shared_ptr<Type> innerType() {
		return elem_type;
	}


private:
	shared_ptr<Type> elem_type;
};

template<class T> class AbstractType: public Type {
public:
	AbstractType( map<string, T> vv ) {
		elem_type = vv;
	}

	virtual shared_ptr<Value> createValue( shared_ptr<Value> in ) {
		return in;
	}

	virtual bool castsTo( const Type &other ) const {
		// record cast must be exact
		if ( other.isRecord() ) {
			AbstractType<T> &other_list = (AbstractType<T> &)other;
			return elem_type == other_list.elem_type;
		}
		return false;
	}

	virtual bool contains( const Type &other ) const {
		return (*this) == other;
	}

	virtual bool operator==( const Type &other ) const {
		if ( other.isRecord() ) {
			AbstractType<T> &other_list = (AbstractType<T> &)other;
			return elem_type == other_list.elem_type;
		}
		return false;
	}

	virtual bool isNull() const {
		return false;
	}

	virtual bool isAtomic() const {
		return false;
	}

	virtual bool isUnion() const {
		return false;
	}

	virtual bool isList() const {
		return false;
	}

	virtual bool isRecord() const {
		return true;
	}

	virtual string nameStr() const {
		return "{record type}";
	}

	T memberType(string name) {
		return elem_type[name];
	}

private:
	map<string, T> elem_type;
};

class UnionType: public Type {
public:
	UnionType( shared_ptr<Type> a, shared_ptr<Type> b ) {
		type_a = a;
		type_b = b;
	}

	virtual shared_ptr<Value> createValue( shared_ptr<Value> v ) {
		return v;
	}

	virtual bool castsTo( const Type &other ) const {
		return type_a->castsTo(other) || type_b->castsTo(other);
	}

	virtual bool contains( const Type &other ) const {
		if (type_a->contains(other) || type_b->contains(other)) return true;
		if ( *this == other ) return true;
		return false;
	}

	// TODO heirachy means sets of more than 2 could be inequal
	virtual bool operator==( const Type &other ) const {
		if ( other.isUnion() ) {
			UnionType &o = (UnionType &)other;
			return (*type_a == *o.type_a && *type_b == *o.type_b)
					|| (*type_a == *o.type_b && *type_b == *o.type_a);
		}
		return false;
	}

	virtual bool isNull() const {
		return false;
	}

	virtual bool isAtomic() const {
		return false;
	}

	virtual bool isUnion() const {
		return true;
	}

	virtual bool isList() const {
		return false;
	}

	virtual bool isRecord() const {
		return false;
	}

	virtual string nameStr() const {
		return type_a->nameStr()+"|"+type_b->nameStr();
	}

private:
	shared_ptr<Type> type_a;
	shared_ptr<Type> type_b;
};

} /* namespace std */

#endif /* TYPE_H_ */
