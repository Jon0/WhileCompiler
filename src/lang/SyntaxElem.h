#ifndef SYNTAX_H_
#define SYNTAX_H_

#include <memory>
#include <vector>

#include "../io/Token.h"

namespace std {

class Type;
class Value;

class SyntaxElem {
public:
	SyntaxElem() {}
	virtual ~SyntaxElem() {}

	void addToken(Token t) {
		tok.push_back(t);
	}

	vector<Token> &getTokens() {
		return tok;
	}

	void copyTokens(SyntaxElem &other) {
		tok.insert(tok.end(), other.tok.begin(), other.tok.end());
	}

	int tokenSize() {
		return tok.size();
	}

private:
	vector<Token> tok;
};

class Type: public SyntaxElem, public enable_shared_from_this<Type> {
public:
	virtual ~Type() {}

	/*
	 * create a related value, casting where needed.
	 * The type of the output value is equal to the
	 * type it is called on.
	 *
	 */
	virtual shared_ptr<Value> createValue( shared_ptr<Value> ) = 0;

	virtual shared_ptr<Type> data() const = 0;

	/*
	 * creates equal type but with a given alias
	 */
	virtual shared_ptr<Type> makeAlias(string) const = 0;

	virtual string nameStr() const = 0;

	virtual string aliasStr() const {
		if (alias.length() > 0) {
			return alias;
		}
		else {
			return nameStr();
		}
	}

	virtual bool castsTo( const Type &other ) const = 0;
	virtual bool contains( const Type &other ) const = 0;
	virtual bool operator==( const Type &other ) const = 0;
	virtual bool operator!=( const Type &other ) const {
		return !(*this == other);
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

	void setAlias(string s) {
		alias = s;
	}

private:
	string alias;

};

class Value: public SyntaxElem, public enable_shared_from_this<Value> {
public:
	Value(shared_ptr<Type> t): type_in(t) {}

	virtual ~Value() {}

	shared_ptr<Type> type() const {
		return type_in;
	}

	virtual shared_ptr<Value> clone( shared_ptr<Type> t ) = 0;

	virtual shared_ptr<Value> clone() {
		return clone(type_in);
	}

	virtual string asString() const = 0;

	virtual bool operator==( const Value &other ) const = 0;

	virtual bool operator!=( const Value &other ) const {
		return !(*this == other);
	}



private:
	shared_ptr<Type> type_in;
};

typedef vector<shared_ptr<Value>> Stack;
typedef vector<shared_ptr<Value>> ValueList;
typedef map<string, shared_ptr<Value>> ValueRecord;

} /* namespace std */

#endif /* SYNTAX_H_ */
