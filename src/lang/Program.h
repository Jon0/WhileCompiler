/*
 * Program.h
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#ifndef PROGRAM_H_
#define PROGRAM_H_

#include <string>
#include <unordered_set>
#include <vector>

#include "Func.h"
#include "SyntaxElem.h"

namespace std {

typedef map<string, Func> FuncMap;

class Program: public SyntaxElem {
public:
	Program( FuncMap );
	virtual ~Program();

	void run();
	void typeCheck();

private:
	FuncMap funcs;

};

} /* namespace std */

#endif /* PROGRAM_H_ */
