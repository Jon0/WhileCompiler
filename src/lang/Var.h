/*
 * Var.h
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#ifndef VAR_H_
#define VAR_H_

#include <memory>
#include <string>

#include "../io/Token.h"
#include "Common.h"
#include "Type.h"

namespace std {

/*
 * a type and name
 */
class Var {
public:
	Var();
	Var(shared_ptr<Type>, Token);
	virtual ~Var();

	string name() const;
	Token token() const;
	shared_ptr<Type> type() const;

	// TODO type check breaks casting
	bool operator==( const Var &other ) const {
			return name() == other.name() && *type() == *other.type();
	}

private:
	shared_ptr<Type> type_info;
	Token in_token;
};

struct VarCompare {
	bool operator()( const Var &first , const Var &second) const {
		return first.name().compare(second.name()) < 0? true: false;
	}
};

typedef map<Var, shared_ptr<Value>, VarCompare> VarMap;
typedef AbstractType<Var> RecordType;

} /* namespace std */

#endif /* VAR_H_ */
