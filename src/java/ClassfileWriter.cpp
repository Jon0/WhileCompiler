/*
 * ClassfileWriter.cpp
 *
 *  Created on: 9/05/2014
 *      Author: remnanjona
 */

#include "Classfile.h"
#include "ClassfileWriter.h"


namespace std {

ClassfileWriter::ClassfileWriter(shared_ptr<Classfile> p, string dir, string n) {
	program = p;
	dirname = dir;
	if (n.length() > 0) {
		fname = n;
	}
	else {
		fname = program->classname();
	}

	/* version */
	version_major = 49;
	version_minor = 0;
}

ClassfileWriter::ClassfileWriter(string name): outfile(name, ofstream::binary){
	/* version */
	version_major = 49;
	version_minor = 0;

	/* file header */
	write_u4(0xCAFEBABE);
	write_u2(version_minor);
	write_u2(version_major);
}

ClassfileWriter::~ClassfileWriter() {
	outfile.close();
}

void ClassfileWriter::writeClassfile() {
	outfile.open( filepath() );

	/* file header */
	write_u4(0xCAFEBABE);
	write_u2(version_minor);
	write_u2(version_major);

	vector<unsigned char> bytes = program->getBytes();
	for (unsigned char c: bytes) {
		write_u1( c );
	}

}

void ClassfileWriter::write_u4(unsigned int i) {
	outfile << (unsigned char) (i >> 24);
	outfile << (unsigned char) (i >> 16);
	outfile << (unsigned char) (i >> 8);
	outfile << (unsigned char) (i >> 0);
}

void ClassfileWriter::write_u2(unsigned short i) {
	outfile << (unsigned char) (i >> 8);
	outfile << (unsigned char) (i >> 0);
}

void ClassfileWriter::write_u1(unsigned char i) {
	outfile << (unsigned char) i;
}

void ClassfileWriter::write_str(string i) {
	for (unsigned char c: i) {
		write_u1(c);
	}
}

string ClassfileWriter::dirctoryname() {
	return dirname;
}

string ClassfileWriter::filename() {
	return fname;
}

string ClassfileWriter::filepath() {
	return dirctoryname() + filename()+".class";
}

} /* namespace std */
