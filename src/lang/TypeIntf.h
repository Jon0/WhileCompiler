#ifndef TYPEINTF_H_
#define TYPEINTF_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "SyntaxElem.h"
#include "ValueIntf.h"

namespace std {

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

	virtual void visit(shared_ptr<SyntaxVisitor> v) {
		v->accept( shared_from_this() );
	}

private:
	string alias;

};

} /* namespace std */

#endif /* TYPEINTF_H_ */
