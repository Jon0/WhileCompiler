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
#include "TypeIntf.h"
#include "Value.h"

namespace std {

class UnknownType;

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
 * only atomic types are supported
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
		else {
			return Func<int, Return>::func( in );
		}
	}
};

class UnknownType: public Type {

	virtual shared_ptr<Value> createValue( shared_ptr<Value> v ) {
		return v;
	}

	virtual shared_ptr<Type> data() const {
		return shared_ptr<Type>(new UnknownType(*this));
	}

	virtual shared_ptr<Type> makeAlias(string aname) const {
		shared_ptr<Type> t = data();
		t->setAlias(aname);
		return t;
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

	virtual string nameStr() const {
		return "unknown";
	}
};

class VoidType: public Type {

	virtual shared_ptr<Value> createValue( shared_ptr<Value> v ) {
		return v;
	}

	virtual shared_ptr<Type> data() const {
		return shared_ptr<Type>(new VoidType(*this));
	}

	virtual shared_ptr<Type> makeAlias(string aname) const {
		shared_ptr<Type> t = data();
		t->setAlias(aname);
		return t;
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

	virtual string nameStr() const {
		return "void";
	}
};

class NullType: public Type {

	virtual shared_ptr<Value> createValue( shared_ptr<Value> v ) {
		return v;
	}

	virtual shared_ptr<Type> data() const {
		return shared_ptr<Type>(new NullType(*this));
	}

	virtual shared_ptr<Type> makeAlias(string aname) const {
		shared_ptr<Type> t = data();
		t->setAlias(aname);
		return t;
	}

	virtual bool castsTo( const Type &other ) const {
		return false;
	}

	virtual bool contains( const Type &other ) const {
		return other.isNull();
	}

	virtual bool operator==( const Type &other ) const {
		return other.isNull();
	}

	virtual bool isNull() const {
		return true;
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

	virtual shared_ptr<Type> data() const {
		return shared_ptr<Type>(new AtomicType(*this));
	}

	virtual shared_ptr<Type> makeAlias(string aname) const {
		shared_ptr<Type> t = data();
		t->setAlias(aname);
		return t;
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

	virtual bool isAtomic() const {
		return true;
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
		is_string = false;
		elem_type = NULL;
	}

	ListType(shared_ptr<Type> n) {
		is_string = false;
		elem_type = n;
	}

	ListType(shared_ptr<Type> n, bool isStr) {
		is_string = isStr;
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

	virtual shared_ptr<Type> data() const {
		return shared_ptr<Type>(new ListType(*this));
	}

	virtual shared_ptr<Type> makeAlias(string aname) const {
		shared_ptr<Type> t = data();
		t->setAlias(aname);
		return t;
	}

	virtual bool castsTo( const Type &other ) const {
		if ( other.isList() ) {
			ListType &other_list = (ListType &)other;
			return elem_type->castsTo(*other_list.elem_type);
		}
		return false;
	}



	virtual bool contains( const Type &other ) const;

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

	virtual bool isList() const {
		return true;
	}

	virtual string nameStr() const {
		if (is_string) {
			return "string";
		}
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

	bool isString() {
		return is_string;
	}


private:
	bool is_string;
	shared_ptr<Type> elem_type;
};

class UnionType: public Type {
public:
	UnionType( vector<shared_ptr<Type>> t ) {

		// format and nomalise
		vector<shared_ptr<Type>> ntypes;
		for (auto &tp: t) {
			if (tp->isUnion()) {
				UnionType &o = (UnionType &)tp;
				for (auto &otp: o.types) {
					bool contains = false;
					for (shared_ptr<Type> itp: ntypes) {
						if (itp->contains(*otp)) contains = true;
						break;
					}
					if (!contains) ntypes.push_back(otp);
				}
			}
			else {
				bool contains = false;
				for (shared_ptr<Type> itp: ntypes) {
					if (itp->contains(*tp)) contains = true;
					break;
				}
				if (!contains) ntypes.push_back(tp);
			}
		}

		str = "";
		int i = 0;
		for (auto a: ntypes) {
			str += a->nameStr();
			if (i < ntypes.size() - 1) str += "|";
			i++;
		}

		types = normalise(ntypes);
	}

	virtual shared_ptr<Value> createValue( shared_ptr<Value> v ) {
		if ( v->type()->isUnion() ) {
			throw runtime_error("union type should not occur on value instance");
		}
		return v;
	}

	virtual shared_ptr<Type> data() const {
		if (types.size() == 1) return types[0];
		else {
			return shared_ptr<Type>(new UnionType(*this));
		}
	}

	virtual shared_ptr<Type> makeAlias(string aname) const {
		shared_ptr<Type> t = data();
		t->setAlias(aname);
		return t;
	}

	virtual bool castsTo( const Type &other ) const {
		if (data()->contains(other)) return true;
		bool result = false;
		for (auto a: types) result |= a->castsTo(other);
		return result;
	}

	virtual bool contains( const Type &other ) const {
		// TODO when other is also a union
		bool result = (*data() == other);
		for (auto a: types) result |= a->contains(other);
		return result;
	}

	virtual bool operator==( const Type &other ) const {
		if ( other.data()->isUnion() ) {
			UnionType &o = (UnionType &)other;

			// each value must equal a value in other
			bool matches_all = true;
			for (auto a: types) {
				bool matches_one = false;
				for (auto b: o.types) matches_one |= (*a == *b);
				matches_all &= matches_one;
			}
			return matches_all;
		}
		return false;
	}

	virtual bool isUnion() const {
		return true;
	}

	virtual string nameStr() const {
		return str;
	}

	bool containedByList( const Type &other ) {
		bool result = true;
		for (auto a: types) {
			if (!a->isList()) return false;

			ListType &a_list = (ListType &)*a;

			result &= other.contains(*a_list.innerType());
		}
		return result;
	}

private:
	string str;
	vector<shared_ptr<Type>> types;

	vector<shared_ptr<Type>> normalise( const vector<shared_ptr<Type>> );

};

template<class T> class AbstractType: public Type {
public:
	AbstractType( map<string, T> vv ) {
		elem_type = vv;
	}

	virtual shared_ptr<Value> createValue( shared_ptr<Value> in ) {
		shared_ptr<TypedValue<ValueRecord>> iptr = static_pointer_cast<TypedValue<ValueRecord>, Value>( in );

		// cast each value in record
		ValueRecord newRecord;
		for ( auto entry : elem_type ) {
			shared_ptr<Value> newEntry = entry.second.type()->createValue( iptr->value()[entry.first] );
			newRecord.insert( ValueRecord::value_type(entry.first, newEntry) );
		}
		return make_shared<TypedValue<ValueRecord>>( shared_from_this(), newRecord );
	}

	virtual shared_ptr<Type> data() const {
		return shared_ptr<Type>(new AbstractType(*this));
	}

	virtual shared_ptr<Type> makeAlias(string aname) const {
		shared_ptr<Type> t = data();
		t->setAlias(aname);
		return t;
	}

	virtual bool castsTo(const Type &other) const {
		if (contains(*other.data()))
			return true;

		if (other.data()->isRecord()) {
			AbstractType<T> &other_list = (AbstractType<T> &) other;
			if (other_list.elem_type.size() != elem_type.size()) return false;

			// names are the same, and inner types cast
			bool canCast = true;
			for (auto &entry : elem_type) {
				auto it = other_list.elem_type.find(entry.first);
				if (it == other_list.elem_type.end()) {
					return false;
				} else {
					canCast &= entry.second.type()->castsTo(
							*(*it).second.type());
				}
			}

			return canCast;
		}
		return false;
	}

	virtual bool contains(const Type &other) const {
		if (other.data()->isRecord()) {
			AbstractType<T> &other_list = (AbstractType<T> &) other;
			if (other_list.elem_type.size() != elem_type.size()) return false;

			// names are the same, and inner types contain
			bool conts = true;
			for (auto &entry : elem_type) {
				auto it = other_list.elem_type.find(entry.first);
				if (it == other_list.elem_type.end()) {
					return false;
				} else {
					conts &= entry.second.type()->contains(
							*(*it).second.type());
				}
			}

			return conts;
		}
		return false;
	}

	virtual bool operator==( const Type &other ) const {
		if ( other.data()->isRecord() ) {
			AbstractType<T> &other_list = (AbstractType<T> &)other;
			return elem_type == other_list.elem_type;
		}
		return false;
	}

	virtual bool isRecord() const {
		return true;
	}

	virtual string nameStr() const {
		string s = "{";

		// cast each value in record
		int i = 0;
		for ( auto entry : elem_type ) {
			s += entry.second.type()->nameStr();
			s += " ";
			s += entry.first;
			if (i < elem_type.size() - 1) s += ",";
			i++;
		}
		s += "}";
		return s;
	}

	T memberType(string name) {
		return elem_type[name];
	}

	bool sameNames(const Type &other) {
		if ( other.data()->isRecord() ) {
			AbstractType<T> &other_rec = (AbstractType<T> &)other;
			if (other_rec.elem_type.size() != elem_type.size()) return false;

			bool same = true;
			for (auto &entry : elem_type) {
				auto it = other_rec.elem_type.find(entry.first);
				if (it == other_rec.elem_type.end()) {
					return false;
				} else {
					same &= entry.second.name() == (*it).second.name();
				}
			}
			return same;
		}
		return false;
	}

	map<string, T> getElems() {
		return elem_type;
	}

	void modify(map<string, T> e) {
		elem_type = e;
	}

private:
	map<string, T> elem_type;
};

} /* namespace std */

#endif /* TYPE_H_ */
