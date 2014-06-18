/*
 * JavaProgram.h
 *
 *  Created on: 19/06/2014
 *      Author: asdf
 */

#ifndef JAVAPROGRAM_H_
#define JAVAPROGRAM_H_

#include <string>
#include <memory>
#include <vector>

namespace std {

class Classfile;

class JavaProgram {
public:
	JavaProgram();
	virtual ~JavaProgram();

	string getName();
	shared_ptr<Classfile> getClassfile();

	void beginFunction( string );

private:
	shared_ptr<Classfile> classfile;

};

} /* namespace std */

#endif /* JAVAPROGRAM_H_ */
