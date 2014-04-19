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
