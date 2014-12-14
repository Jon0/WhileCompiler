#ifndef VALUEINTF_H_
#define VALUEINTF_H_

namespace lang {
using namespace std;

class Type;

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

	virtual void visit(shared_ptr<SyntaxVisitor> v) {
		return v->accept( shared_from_this() );
	}

private:
	shared_ptr<Type> type_in;
};

typedef vector<shared_ptr<Value>> Stack;
typedef vector<shared_ptr<Value>> ValueList;
typedef map<string, shared_ptr<Value>> ValueRecord;

} /* namespace lang */

#endif /* VALUEINTF_H_ */
