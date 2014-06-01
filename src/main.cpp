#include <iostream>

#include "io/Directory.h"
#include "io/Lexer.h"
#include "io/Parser.h"

#include "lang/Common.h"

#include "x86/WhileToX86.h"
#include "x86/X86Program.h"
#include "x86/X86Writer.h"

#include "test/Test.h"
#include "test/InterfaceTest.h"

using namespace std;

bool debug = true;

int main(int argc, char *argv[]) {
	if (argc == 1) {
		cout << "requires a file" << endl;
		return 0;
	}

	bool test = false, testI = false, debug = false;
	string testDir;
	string filename;
	for (int i = 1; i < argc; ++i) {
		string s = argv[i];
		if (s == "--test") {
			test = true;
			testDir = argv[++i];
		}
		else if (s == "--testinterface") {
			testI = true;
		}
		else if (s == "--debug") {
			debug = true;
		}
		else  {
			filename = s;
		}
	}

	if (test) {
		string directoryPath = argv[2];

		// get list of files in test folder
		Directory dir(directoryPath);
		vector<string> wfiles = dir.fileList("while");

		shared_ptr<Test> t = make_shared<Test>();
		t->testDirectory(directoryPath, wfiles);
		return 0;
	}
	else if (testI) {
		shared_ptr<InterfaceTest> t = make_shared<InterfaceTest>();
		return 0;
	}

	try {
		// read while program
		Lexer lex(filename.c_str());
		Parser parser(lex);
		shared_ptr<Program> input = parser.read();

		// convert to a x86 assembly program
		shared_ptr<X86Program> x86prog = make_shared<X86Program>();
		shared_ptr<WhileToX86> converter = make_shared<WhileToX86>(x86prog, debug);
		input->visit(converter);

		// save executable
		shared_ptr<X86Writer> writer = make_shared<X86Writer>(x86prog, "bin/", "");
		writer->writeExecutable();

	}
	catch (exception &e) {
		cout << "error " << e.what() << endl;
	}





}
