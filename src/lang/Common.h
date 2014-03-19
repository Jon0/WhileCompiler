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

#include "Var.h"
#include "Value.h"

namespace std {

typedef vector<shared_ptr<Value>> Stack;
typedef map<Var, shared_ptr<Value>, VarCompare> VarMap;

} /* namespace std */

#endif /* COMMON_H_ */
