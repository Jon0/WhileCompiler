/*
 * ClassfileWriter.h
 *
 *  Created on: 9/05/2014
 *      Author: remnanjona
 */

#ifndef CLASSFILEWRITER_H_
#define CLASSFILEWRITER_H_

#include <fstream>
#include <memory>
#include <string>

namespace std {

class Classfile;

class ClassfileWriter {
public:
	ClassfileWriter(shared_ptr<Classfile>, string, string);
	ClassfileWriter(string); // no longer used
	virtual ~ClassfileWriter();

	void writeClassfile();

	void write_u4(unsigned int i);
	void write_u2(unsigned short i);
	void write_u1(unsigned char i);
	void write_str(string i);

	string dirctoryname();
	string filename();
	string filepath();

private:
	shared_ptr<Classfile> program;
	string dirname, fname;

	ofstream outfile;
	unsigned short version_major, version_minor;
};

} /* namespace std */

#endif /* CLASSFILEWRITER_H_ */
