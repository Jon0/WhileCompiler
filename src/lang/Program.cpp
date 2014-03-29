/*
 * Program.cpp
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#include "Program.h"

namespace std {

Program::Program( FuncMap f ) {
	funcs = f;
}

Program::~Program() {}

void Program::run() {
	Stack s;

	FuncMap::iterator i = funcs.find( "main" );
	if (i == funcs.end()) {
		throw runtime_error("no main function found");
	}
	else {
		(*i).second.execute(s);
	}
}

void Program::typeCheck() {
	for (FuncMap::value_type &f: funcs) {
		f.second.typeCheck();
	}
}

} /* namespace std */
