/*
 * ClassfileWriter.cpp
 *
 *  Created on: 9/05/2014
 *      Author: remnanjona
 */

#include "ClassfileWriter.h"

namespace std {

ClassfileWriter::ClassfileWriter(string name): outfile(name, ofstream::binary){

}

ClassfileWriter::~ClassfileWriter() {
	outfile.close();
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

} /* namespace std */
