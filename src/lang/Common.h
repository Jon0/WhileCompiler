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

namespace lang {

extern bool debug;

class Type;

struct StmtStatus {
	bool isReturn;
	bool isBreak;
};

// TODO move to interpreter

struct AssignState {
	bool defAssign;
	std::shared_ptr<Type> type;
};

struct CheckState {
	std::map<std::string, AssignState> assigned; // TODO scope
	std::shared_ptr<Type> to_return;
	bool returned;
};


} /* namespace lang */

#endif /* COMMON_H_ */
