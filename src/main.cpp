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
		shared_ptr<SyntaxElem> p = parser.read();

		shared_ptr<Bytecode> compiler = shared_ptr<Bytecode>(new Bytecode());

		p->visit(compiler);

		Classfile &cf = compiler->getClassFile();
		cf.write("test.class");
	}
	catch (exception &e) {
		cout << e.what() << endl;
	}
}
