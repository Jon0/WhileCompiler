/*
 * X86Writer.h
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#ifndef X86WRITER_H_
#define X86WRITER_H_

#include <memory>
#include <string>
#include <fstream>

namespace std {

class X86Program;

class X86Writer {
public:
	X86Writer(shared_ptr<X86Program>, string, string);
	virtual ~X86Writer();

	string filepath();

	/* write functions return path to output */
	string writeAssembly();
	string writeExecutable();

private:
	shared_ptr<X86Program> program;
	string directory, filename;
};

} /* namespace std */

#endif /* X86WRITER_H_ */
