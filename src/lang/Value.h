/*
 * Value.h
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#ifndef VALUE_H_
#define VALUE_H_

#include <iostream>
#include <memory>
#include <map>
#include <string>

#include "Common.h"
#include "SyntaxElem.h"

namespace std {

class NullValue: public Value {
public:
	NullValue(shared_ptr<Type> t): Value( t ) {}

	virtual shared_ptr<Value> clone( shared_ptr<Type> t ) {
		return t->createValue( shared_from_this() );
	}

	virtual string asString() const {
		return "null";
	}

	virtual bool operator==( const Value &other ) const {
		return (*other.type() == *type());
	}
};

template<class T> class TypedValue: public Value {
public:
	TypedValue(shared_ptr<Type> t, T val): Value(t) {
		internal_type = val;
	}

	virtual ~TypedValue() {}

	T &value() {
		return internal_type;
	}

	virtual shared_ptr<Value> clone( shared_ptr<Type> t ) {
		return t->createValue( shared_from_this() );
	}

	virtual string asString() const {
		return conv( internal_type );
	}

	virtual bool operator==( const Value &other ) const {
		if ( type()->contains(*other.type()) || other.type()->contains(*type()) ) {
			TypedValue<T> &other_v = (TypedValue<T> &)other;

			if ( type()->isAtomic() ) {
				return internal_type == other_v.value();
			}
			else if ( type()->isList() && other.type()->isList() ) {
				ValueList &va = (ValueList &)internal_type;
				ValueList &vb = ((TypedValue<ValueList> &)other).value();
				if ( va.size() != vb.size() ) return false;
				for (int i = 0; i < va.size(); ++i) {
					if (!(*(va[i]) == *(vb[i]))) return false;
				}
				return true;
			}
			else if ( type()->isRecord() && other.type()->isRecord() ) {
				ValueRecord &va = (ValueRecord &)internal_type;
				ValueRecord &vb = ((TypedValue<ValueRecord> &)other).value();
				if ( va.size() != vb.size() ) return false;
				for (ValueRecord::value_type t: va) {
					if (vb.count(t.first) == 0 ||
							!( *(*vb.find(t.first)).second == *t.second) ) return false;
				}
				return true;
			}
			// union types should not occur in instances except lists
		}
		return false;
	}

	template<class R> R asType() const {
		return cast( internal_type );
	}

private:
	T internal_type;

	template<class R, class I> R cast(I s) const {
		return (R) s;
	}

	string conv(char s) const {
		return string(&s, 1);
	}

	string conv(string s) const {
		return s;
	}

	string conv(bool s) const {
		return s? "true": "false";
	}

	string conv(int s) const {
		return to_string(s);
	}

	string conv(double s) const {
		char buf[64];
		sprintf(buf, "%g", s); // %.2f
		string str_result(buf);
		if (str_result.find(".") == string::npos
				&& str_result.find("e") == string::npos) {
			str_result += ".0";
		}
		return str_result;
	}

	string conv(ValueList s) const {
		string result = "";
		if (type()->nameStr() == "string") {
			for (shared_ptr<Value> v : s) {
				result += v->asString();
			}
		}
		else {
			result += "[";
			int i = 0;
			for (shared_ptr<Value> v : s) {
				result += v->asString();
				if (i++ < s.size() - 1)
					result += ", ";
			}
			result += "]";
		}
		return result;
	}

	string conv(ValueRecord s) const {
		string result = "{";
		int i = 0;
		for (ValueRecord::value_type &v: s) {
			result += v.first;
			result += ":";
			result += v.second->asString();
			if ( i++ < s.size()-1 ) result += ",";
		}
		result += "}";

		return result;
	}

};

template<class T> shared_ptr<Value> makeValue(shared_ptr<Type> t, T in) {
	return shared_ptr<Value>(new TypedValue<T>( t, in ));
}

} /* namespace std */

#endif /* VALUE_H_ */
