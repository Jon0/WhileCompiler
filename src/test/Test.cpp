/*
 * Test.cpp
 *
 *  Created on: 9/04/2014
 *      Author: remnanjona
 */

#include <streambuf>
#include <vector>
#include <memory>

#include "../io/Lexer.h"
#include "../io/Parser.h"
#include "../io/Pipe.h"
#include "../lang/Common.h"
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
			break;
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
		Lexer lex(in+inExt);
		Parser parser(lex);
		shared_ptr<Program> input = parser.read();

		// convert to a x86 assembly program
		shared_ptr<X86Program> x86prog = make_shared<X86Program>();
		shared_ptr<WhileToX86> converter = make_shared<WhileToX86>(x86prog);
		input->visit(converter);

		// save executable
		shared_ptr<X86Writer> writer = make_shared<X86Writer>(x86prog, "bin/", "");
		writer->writeExecutable();

		// run program record output
		Pipe p;
		string outStr = p.exec("./"+writer->filepath());

		// read expected output
		ifstream t(in+cmpExt);
		string expected((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());

		// compare to expected output
		//expected = expected.substr(0, expected.length() - 1); // buffer ends with "\r"
		if (outStr != expected) {
			cout << endl;
			cout << "test failed - " << in << endl;
			cout << "have:\t" << outStr << endl;
			cout << "expect:\t" << expected << endl;
			cout << "----------------------" << endl;
		}
		else {
			cout << "test passed - " << in << endl;
			passed++;
		}
		completed++;

	}
	catch (exception &e) {
		cout << "error in test " << in << ", " << e.what() << endl;
	}
}


} /* namespace std */
