/*
 * Classfile.h
 *
 *  Created on: 2/05/2014
 *      Author: remnanjona
 */

#ifndef CLASSFILE_H_
#define CLASSFILE_H_

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ClassfileWriter.h"
#include "ConstantPool.h"
#include "JavaFunction.h"

namespace std {

class SyntaxElem;

class Classfile: public enable_shared_from_this<Classfile> {
public:
	Classfile();
	Classfile(shared_ptr<Program>);
	virtual ~Classfile();

	void beginFunction( string );

	void read(string);
	void write();

	string classname();
	shared_ptr<ConstantPool> getConstPool();
	vector<unsigned char> getBytes();
	void write_u4(unsigned int i);
	void write_u2(unsigned short i);
	void write_u1(unsigned char i);
	void write_str(string i);

private:
	string class_name;
	shared_ptr<Program> program;
	shared_ptr<ConstantPool> cp;
	vector<unsigned char> bytes;

	map<string, shared_ptr<JavaFunction>> functions;

};

} /* namespace std */

#endif /* CLASSFILE_H_ */
