/*
 * Test.cpp
 *
 *  Created on: 9/04/2014
 *      Author: remnanjona
 */

#include <algorithm>
#include <cstring>
#include <streambuf>
#include <vector>
#include <memory>

#include "../io/Lexer.h"
#include "../io/Parser.h"
#include "../lang/Common.h"

#include "../java/WhileToJava.h"
#include "../java/Classfile.h"
#include "../java/ClassfileWriter.h"
#include "../java/ConstantPool.h"

#include "../x86/WhileToX86.h"
#include "../x86/X86Program.h"
#include "../x86/X86Writer.h"

#include "Test.h"

namespace std {

Test::Test() {
	passed = 0;
	completed = 0;
	total = 0;
}

Test::~Test() {}

void Test::testDirectory(string directory, vector<string> wfiles) {
	passed = 0;
	completed = 0;
	total = wfiles.size();

	for ( string wf: wfiles ) {
		runTest(directory+wf, "");

		if (passed < completed) {
			//break;
		}
	}

	cout << endl;
	cout << "----------------------" << endl;
	cout << passed << " / " << total << " tests passed" << endl;
}

void Test::runTest(string in, string out) {
	cout << "----------------------" << endl;
	cout << "building " << in << endl;
	cout << completed << "/" << total << endl;
	cout << "----------------------" << endl;

	try {
		string inExt = ".while";
		string cmpExt = ".sysout";

		// read while program
		io::parser::Lexer lex(in+inExt);
		io::parser::WhileParser parser(lex);
		string outStr = runJava( parser.read() );

		// read expected output
		ifstream t(in+cmpExt);
		string expected((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());


		// remove return characters from strings
		string outNR = outStr, expectNR = expected;
		char chars[] = "\r\n";
		for (unsigned int i = 0; i < strlen(chars); ++i) {
			outNR.erase(remove(outNR.begin(), outNR.end(), chars[i]), outNR.end());
			expectNR.erase(remove(expectNR.begin(), expectNR.end(), chars[i]), expectNR.end());
		}

		// compare to expected output
		//expected = expected.substr(0, expected.length() - 1); // buffer ends with "\r"
		if (outNR != expectNR) {
			cout << endl;
			cout << "test failed - " << in << endl;
			cout << "have:\n" << outStr << endl;
			cout << "expect:\n" << expected << endl;
			cout << "----------------------" << endl;
		}
		else {
			cout << "test passed - " << in << endl;
			passed++;
		}


	}
	catch (exception &e) {
		cout << "error in test " << in << ", " << e.what() << endl;
	}
	completed++;
}

string Test::runX86(shared_ptr<lang::Program> input) {
	// convert to a x86 assembly program
	shared_ptr<X86Program> x86prog = make_shared<X86Program>();
	shared_ptr<WhileToX86> converter = make_shared<WhileToX86>(x86prog, true);
	input->visit(converter);

	// save executable
	shared_ptr<X86Writer> writer = make_shared<X86Writer>(x86prog, "bin/", "");
	writer->writeExecutable();

	// run program record output
	return p.exec("./"+writer->filepath());
}

string Test::runJava(shared_ptr<lang::Program> program) {
	cout << "writing file " << program->getProgramName() << ".class" << endl;
	ClassfileWriter out( "bin/" + program->getProgramName()+ ".class" );


	return "";
}


} /* namespace std */
