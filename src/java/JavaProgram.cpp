/*
 * JavaProgram.cpp
 *
 *  Created on: 19/06/2014
 *      Author: asdf
 */

#include "Classfile.h"
#include "JavaProgram.h"

namespace std {

JavaProgram::JavaProgram() {
	classfile = make_shared<Classfile>();
}

JavaProgram::~JavaProgram() {}

string JavaProgram::getName() {
	return classfile->classname();
}

shared_ptr<Classfile> JavaProgram::getClassfile() {
	return classfile;
}

void JavaProgram::beginFunction( string s ) {
	classfile->beginFunction(s);
}

} /* namespace std */
