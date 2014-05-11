/*
 * Classfile.h
 *
 *  Created on: 2/05/2014
 *      Author: remnanjona
 */

#ifndef CLASSFILE_H_
#define CLASSFILE_H_

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "ClassfileWriter.h"
#include "ConstantPool.h"

namespace std {

class SyntaxElem;

class Classfile: public enable_shared_from_this<Classfile> {
public:
	Classfile(shared_ptr<Program>);
	virtual ~Classfile();

	void read(string);
	void write();

	ConstantPool &getConstPool();

private:
	string class_name;
	shared_ptr<Program> program;

	unsigned short version_major, version_minor;

	ConstantPool cp;
};

} /* namespace std */

#endif /* CLASSFILE_H_ */
