/*
 * X86Writer.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "../io/Pipe.h"

#include "X86Instruction.h"
#include "X86Program.h"
#include "X86Writer.h"

namespace std {

X86Writer::X86Writer(shared_ptr<X86Program> p, string d, string f) {
	program = p;
	directory = d;
	filename = f;
	if (filename == "") {
		filename = p->getName();
	}
}

X86Writer::~X86Writer() {}

string X86Writer::filepath() {
	return directory+filename;
}

string X86Writer::writeAssembly() {
	string fullpath = directory+filename+".s";
	//cout << "writing assembly to " << fullpath << endl;
	ofstream outfile(fullpath);

	// write each instruction of the program
	vector<string> secs = program->getSections();
	for (string s: secs) {
		vector<shared_ptr<X86Instruction>> ins = program->getInstructions(s);
		for (shared_ptr<X86Instruction> i: ins) {
			outfile << i->str() << endl;
		}
	}
	outfile.close();

	return fullpath;
}

string X86Writer::writeExecutable() {
	string assemblyPath = writeAssembly();

	// use gcc to compile
	io::Pipe p;
	string out = p.exec( "gcc lib/lib.c "+assemblyPath+" -o "+directory+filename );

	return assemblyPath;
}

} /* namespace std */
