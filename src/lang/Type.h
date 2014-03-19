/*
 * Type.h
 *
 *  Created on: 16/03/2014
 *      Author: remnanjona
 */

#ifndef TYPE_H_
#define TYPE_H_

#include <memory>
#include <string>

#include "SyntaxElem.h"

namespace std {

class Type: public SyntaxElem {
public:
	virtual ~Type() {

	}

	//virtual bool operator==( const Type &other ) const = 0;

	virtual bool isList() = 0;
	virtual string nameStr() = 0;
};

template<class T> class AtomicType: public Type {
public:
	typedef T InternalType;

	AtomicType(string n) {
		name = n;
	}

	virtual ~AtomicType() {

	}

//	bool operator==( const AtomicType &other ) const {
//			return name == other.name; // TODO check types
//	}

	virtual bool isList() {
		return false;
	}

	virtual string nameStr() {
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

	virtual bool isList() {
		return true;
	}

	virtual string nameStr() {
		return "[" + elem_type->nameStr() + "]";
	}


private:
	shared_ptr<Type> elem_type;
};

class RecordType: public Type {
public:
	RecordType(shared_ptr<Type> n) {
		elem_type = n;
	}

	virtual bool isList() {
		return true;
	}

	virtual string nameStr() {
		return "{" + elem_type->nameStr() + "}";
	}


private:
	shared_ptr<Type> elem_type;
};

} /* namespace std */

#endif /* TYPE_H_ */
