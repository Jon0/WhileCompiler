#include <iostream>

#include "io/Directory.h"
#include "io/Lexer.h"
#include "io/Parser.h"

#include "lang/Common.h"

#include "java/Bytecode.h"

#include "test/Test.h"
#include "test/InterfaceTest.h"

using namespace std;

bool debug = true;

int main(int argc, char *argv[]) {
	if (argc == 1) {
		cout << "requires a file" << endl;
		return 0;
	}

	string arg1 = argv[1];
	if (arg1 == "--test") {
		string directoryPath = argv[2];

		// get list of files in test folder
		Directory dir(directoryPath);
		vector<string> wfiles = dir.fileList("while");

		shared_ptr<Test> t = make_shared<Test>();
		t->testDirectory(directoryPath, wfiles);
		return 0;
	}
	else if (arg1 == "--testinterface") {
		shared_ptr<InterfaceTest> t = make_shared<InterfaceTest>();
		return 0;
	}

	try {
		Lexer lex(argv[1]);
		Parser parser(lex);
		shared_ptr<Program> p = parser.read();


		Classfile cf(p);
		cf.write();
	}
	catch (exception &e) {
		cout << e.what() << endl;
	}
}
