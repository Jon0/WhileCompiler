#ifndef SYNTAX_H_
#define SYNTAX_H_

#include <memory>
#include <vector>

namespace std {

class Type;
class Value;

class SyntaxElem {
public:
	virtual ~SyntaxElem() {}

	// TODO parent, children
};

class Type: public SyntaxElem, public enable_shared_from_this<Type> {
public:
	virtual ~Type() {

	}

	/*
	 * create a related value, casting where needed.
	 * The type of the output value is equal to the
	 * type it is called on.
	 *
	 */
	virtual shared_ptr<Value> createValue( shared_ptr<Value> ) = 0;

	//TODO subtyping
	virtual bool castsTo( const Type &other ) const = 0;
	virtual bool contains( const Type &other ) const = 0;

	//virtual ptr cast( const value &v ) const = 0;

	//virtual bool element_equals( const value &v ) const = 0;

	virtual bool operator==( const Type &other ) const = 0;

	virtual bool isNull() const =  0;
	virtual bool isAtomic() const = 0;
	virtual bool isUnion() const = 0;
	virtual bool isList() const = 0;
	virtual bool isRecord() const = 0;
	virtual string nameStr() const = 0;
};

class Value: public SyntaxElem, public enable_shared_from_this<Value> {
public:
	Value(shared_ptr<Type> t): type_in(t) {}

	virtual ~Value() {}

	shared_ptr<Type> type() const {
		return type_in;
	}

	/*
	 * remove, doesnt support deep type changing
	 */
	virtual shared_ptr<Value> clone( shared_ptr<Type> t ) = 0;

	virtual string asString() const = 0;

	virtual bool operator==( const Value &other ) const = 0;

private:
	shared_ptr<Type> type_in;
};

typedef vector<shared_ptr<Value>> Stack;
typedef vector<shared_ptr<Value>> ValueList;
typedef map<string, shared_ptr<Value>> ValueRecord;

} /* namespace std */

#endif /* SYNTAX_H_ */
