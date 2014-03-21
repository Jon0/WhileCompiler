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

#include "Common.h"
#include "Type.h"

namespace std {

class Var {
public:
	Var();
	Var(shared_ptr<Type>, string);
	virtual ~Var();

	string name() const;
	shared_ptr<Type> type() const;

	bool operator==( const Var &other ) const {
			return name() == other.name(); // TODO check types
	}

private:
	shared_ptr<Type> type_info;
	string name_str;
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
