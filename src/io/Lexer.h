#ifndef LEXER_H_
#define LEXER_H_

#include <fstream>
#include <iostream>
#include <memory>
#include <queue>

#include "Token.h"

namespace io {
namespace parser {
using namespace std;

/*
 *	break a file into a stream of tokens
 *
 *	punctuation is returned as single char tokens
 *
 *	comments are removed from output
 */
class Lexer {
public:
	Lexer(const char *);
	Lexer(string);
	virtual ~Lexer();

	string getFilename();

	queue<Token> getTokens();

private:
	string filename;
	int current_line;
	string current_line_str;
	int current_line_char;
	ifstream file;

	bool canRead();
	char popChar();
	char peekChar();
	void incLine();

	Token makeToken(string);

	Token getIdentifier();
	Token getNumerical();
	Token getString();


	void skipRestOfComment();
};

} /* namespace parser */
} /* namespace io */

#endif /* LEXER_H_ */
