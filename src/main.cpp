#include <iostream>

#include "io/Lexer.h"
#include "io/Parser.h"

#include "lang/Common.h"

#include "java/Bytecode.h"

#include "test/Test.h"

using namespace std;

bool debug = true;

int main(int argc, char *argv[]) {
	if (argc == 1) {
		cout << "requires a file" << endl;
		return 0;
	}

	string arg1 = argv[1];
	if (arg1 == "--test") {
		shared_ptr<Test> t = make_shared<Test>();

		// list of files in test folder
		//string directory = "tests/mytests/";
		//vector<string> wfiles = {"first", "second", "print2"};

		string directory = "tests/mytests/";
		vector<string> wfiles = {"funccall"};

		t->testDirectory(directory, wfiles);
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
