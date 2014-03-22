#ifndef LEXER_H_
#define LEXER_H_

#include <fstream>
#include <iostream>
#include <queue>

#include "Token.h"

namespace std {

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
	virtual ~Lexer();

	queue<Token> getTokens();

private:
	int current_line;
	ifstream file;

	Token getIdentifier();
	Token getNumerical();
	Token getString();

	void skipWhiteSpace();
	void skipRestOfLine();
	void skipRestOfComment();
};

} /* namespace std */

#endif /* LEXER_H_ */
