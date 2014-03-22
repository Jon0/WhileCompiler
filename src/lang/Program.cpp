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
	(*i).second.execute(s);
}

} /* namespace std */
