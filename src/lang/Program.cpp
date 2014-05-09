/*
 * Program.cpp
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#include "Program.h"

namespace std {

Program::Program( FuncMap f, string pn ) {
	funcs = f;
	program_name = pn;
}

Program::~Program() {}

string Program::getProgramName() const {
	return program_name;
}

FuncMap Program::getFuncMap() const {
	return funcs;
}

void Program::run() {

}

void Program::typeCheck() {
	for (FuncMap::value_type &f: funcs) {
		f.second->typeCheck();
	}
}

void Program::visitFunc(string s, shared_ptr<SyntaxVisitor> v) {
	FuncMap::iterator i = funcs.find( s );
	if (i == funcs.end()) {
		throw runtime_error("no function named "+s+" found");
	}
	else {
		v->accept( (*i).second );
	}
}

} /* namespace std */
