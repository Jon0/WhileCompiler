#include <iostream>

#include "io/Lexer.h"
#include "io/Parser.h"

using namespace std;

int main(int argc, char *argv[]) {
	if (argc == 1) {
		cout << "requires a file" << endl;
		return 0;
	}

	Lexer lex(argv[1]);
	Parser parser(lex);

	Program p = parser.read();
	p.run();
}
