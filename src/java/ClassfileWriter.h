/*
 * ClassfileWriter.h
 *
 *  Created on: 9/05/2014
 *      Author: remnanjona
 */

#ifndef CLASSFILEWRITER_H_
#define CLASSFILEWRITER_H_

#include <fstream>
#include <string>

namespace std {

class ClassfileWriter {
public:
	ClassfileWriter(string);
	virtual ~ClassfileWriter();

	void write_u4(unsigned int i);
	void write_u2(unsigned short i);
	void write_u1(unsigned char i);
	void write_str(string i);

private:
	ofstream outfile;
	unsigned short version_major, version_minor;
};

} /* namespace std */

#endif /* CLASSFILEWRITER_H_ */
