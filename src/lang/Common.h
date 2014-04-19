/*
 * common.h
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <map>
#include <string>

#include "SyntaxElem.h"

namespace std {

extern bool debug;

class Type;

struct StmtStatus {
	bool isReturn;
	bool isBreak;
};

// TODO move to interpreter

struct AssignState {
	bool defAssign;
	shared_ptr<Type> type;
};

struct CheckState {
	map<string, AssignState> assigned; // TODO scope
	shared_ptr<Type> to_return;
	bool returned;
};


} /* namespace std */

#endif /* COMMON_H_ */
