#ifndef VAR_H_
#define VAR_H_

#include <memory>
#include <string>

#include "../io/Token.h"
#include "Common.h"
#include "Type.h"

namespace lang {
using namespace std;

/*
 * a type and name pair
 */
class Var {
public:
	Var();

	/**
	 * creates a type name pair
	 */
	Var(shared_ptr<Type> type, io::parser::Token name);

	/**
	 * takes a token from given variable
	 * to do same as above
	 */
	Var(shared_ptr<Type> type, Var name);
	virtual ~Var();

	string name() const;
	io::parser::Token token() const;
	shared_ptr<Type> type() const;

	// TODO type check breaks casting
	bool operator==( const Var &other ) const {
			return name() == other.name() && *type() == *other.type();
	}

private:
	shared_ptr<Type> type_info;
	io::parser::Token in_token;
};

struct VarCompare {
	bool operator()( const Var &first , const Var &second) const {
		return first.name().compare(second.name()) < 0? true: false;
	}
};

typedef map<Var, shared_ptr<Value>, VarCompare> VarMap;
typedef AbstractType<Var> RecordType;

} /* namespace lang */

#endif /* VAR_H_ */
