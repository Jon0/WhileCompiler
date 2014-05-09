#include <iostream>

#include "io/Lexer.h"
#include "io/Parser.h"

#include "lang/Common.h"

#include "java/Bytecode.h"

using namespace std;

bool debug = true;

int main(int argc, char *argv[]) {
	if (argc == 1) {
		cout << "requires a file" << endl;
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
