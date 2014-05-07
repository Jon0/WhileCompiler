/*
 * Classfile.cpp
 *
 *  Created on: 2/05/2014
 *      Author: remnanjona
 */

#include <fstream>

#include "Classfile.h"

namespace std {

void write_u4(ofstream &outf, unsigned int i) {
	outf << (unsigned char) (i >> 24);
	outf << (unsigned char) (i >> 16);
	outf << (unsigned char) (i >> 8);
	outf << (unsigned char) (i >> 0);
}

void write_u2(ofstream &outf, unsigned short i) {
	outf << (unsigned char) (i >> 8);
	outf << (unsigned char) (i >> 0);
}

void write_u1(ofstream &outf, unsigned char i) {
	outf << (unsigned char) i;
}

Classfile::Classfile() {
	// TODO Auto-generated constructor stub
	version_major = 51;
	version_minor = 0;

}

Classfile::~Classfile() {
	// TODO Auto-generated destructor stub
}

void Classfile::read(string fname) {}

void Classfile::write(string fname) {
	ofstream outfile(fname.c_str(), ofstream::binary);

	write_u4(outfile, 0xCAFEBABE);
	write_u2(outfile, version_minor);
	write_u2(outfile, version_major);

//	u2 constant_pool_count;
	write_u2(outfile, 2);

//	cp_info constant_pool[constant_pool_count-1];
	write_u1(outfile, 7);
	write_u1(outfile, 0);


//	u2 access_flags;
	write_u2(outfile, 1);

//	u2 this_class;
	write_u2(outfile, 1);

//	u2 super_class;
	write_u2(outfile, 0);

//	u2 interfaces_count;
	write_u2(outfile, 0);

//	u2	interfaces[interfaces_count];

//	u2 	fields_count;
	write_u2(outfile, 0);

//	field_info 	fields[fields_count];

//	u2 	methods_count;
	write_u2(outfile, 0);

//	method_info methods[methods_count];


//	u2 	 attributes_count;
	write_u2(outfile, 0);

//	attribute_info 	attributes[attributes_count];


	outfile.close();
}

} /* namespace std */
