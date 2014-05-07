/*
 * Classfile.h
 *
 *  Created on: 2/05/2014
 *      Author: remnanjona
 */

#ifndef CLASSFILE_H_
#define CLASSFILE_H_

#include <string>

namespace std {

class Classfile {
public:
	Classfile();
	virtual ~Classfile();

	void read(string);
	void write(string);

private:
	unsigned short version_major, version_minor;
};

} /* namespace std */

#endif /* CLASSFILE_H_ */
