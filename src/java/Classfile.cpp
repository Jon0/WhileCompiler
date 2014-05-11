/*
 * Classfile.cpp
 *
 *  Created on: 2/05/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "../lang/Program.h"

#include "Bytecode.h"
#include "Classfile.h"

namespace std {

Classfile::Classfile(shared_ptr<Program> p){
	program = p;

	version_major = 49;
	version_minor = 0;
}

Classfile::~Classfile() {
	// TODO Auto-generated destructor stub
}

void Classfile::read(string fname) {}

void Classfile::write() {
	cout << "writing file " << program->getProgramName() << ".class" << endl;
	ClassfileWriter out( program->getProgramName()+ ".class" );

	/* file header */
	out.write_u4(0xCAFEBABE);
	out.write_u2(version_minor);
	out.write_u2(version_major);

	shared_ptr<ConstantPool> cp = make_shared<ConstantPool>();
	program->visit(cp);
	cp->writeByteCode(out);

	shared_ptr<Bytecode> bc = make_shared<Bytecode>(out, *cp);
	program->visit(bc);
}

ConstantPool &Classfile::getConstPool() {
	return cp;
}

} /* namespace std */
