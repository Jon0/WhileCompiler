/*
 * Value.h
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#ifndef VALUE_H_
#define VALUE_H_

#include <memory>
#include <map>
#include <string>

#include "SyntaxElem.h"
#include "Var.h"

namespace std {

class Value: public SyntaxElem {
public:
	Value(shared_ptr<Type> t): type_in(t) {}

	virtual ~Value() {}

	shared_ptr<Type> type() {
		return type_in;
	}

	virtual string asString() = 0;

private:
	shared_ptr<Type> type_in;
};

typedef vector<shared_ptr<Value>> ValueList;

template<class T> class TypedValue: public Value {
public:
	TypedValue(shared_ptr<Type> t, T val): Value(t) {
		internal_type = val;
	}

	virtual ~TypedValue() {}

	T &value() {
		return internal_type;
	}

	virtual string asString() {
		return str_conv( internal_type );
	}

private:
	T internal_type;

	string str_conv(string s) {
		return s;
	}

	string str_conv(bool s) {
		return s? "true": "false";
	}

	string str_conv(int s) {
		return to_string(s);
	}

	string str_conv(ValueList s) {
		string result = "[";
		int i = 0;
		for (shared_ptr<Value> v: s) {
			result += v->asString();
			if ( i++ < s.size()-1 ) result += ", ";
		}
		result += "]";

		return result;
	}

//	template<class T> string str_conv(T s) {
//		return to_string(s);
//	}
};

} /* namespace std */

#endif /* VALUE_H_ */
