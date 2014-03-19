/*
 * Type.h
 *
 *  Created on: 16/03/2014
 *      Author: remnanjona
 */

#ifndef TYPE_H_
#define TYPE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "SyntaxElem.h"

namespace std {

class Type: public SyntaxElem {
public:
	virtual ~Type() {

	}

	//TODO subtyping
	// virtual bool contains( const Type &other ) const = 0;

	//virtual ptr cast( const value &v ) const = 0;

	virtual bool operator==( const Type &other ) const = 0;

	virtual bool isList() const = 0;
	virtual bool isRecord() const = 0;
	virtual string nameStr() const = 0;
};

template<class T> class AtomicType: public Type {
public:
	typedef T InternalType;

	AtomicType(string n) {
		name = n;
	}

	virtual ~AtomicType() {

	}

	bool operator==( const Type &other ) const {
		if ( !other.isList() && !other.isRecord() ) {
			AtomicType &other_at = (AtomicType &)other;
			return name == other_at.name; // TODO check types
		}
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
	ListType(shared_ptr<Type> n) {
		elem_type = n;
	}

	virtual bool operator==( const Type &other ) const {
		if ( other.isList() ) {
			ListType &other_list = (ListType &)other;
			return *elem_type == *other_list.elem_type;
		}
		return false;
	}

	virtual bool isList() const {
		return true;
	}

	virtual bool isRecord() const {
		return false;
	}

	virtual string nameStr() const {
		return "[" + elem_type->nameStr() + "]";
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

	virtual bool operator==( const Type &other ) const {
		if ( other.isRecord() ) {
			AbstractType<T> &other_list = (AbstractType<T> &)other;
			return elem_type == other_list.elem_type; // TODO check types
		}
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

} /* namespace std */

#endif /* TYPE_H_ */
