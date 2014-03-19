#include <stdexcept>

#include "Lexer.h"

namespace std {

Lexer::Lexer(const char *filename):
		file(filename)
{
	if (!file) {
		string error_str = string("cannot find ").append(filename);
		throw invalid_argument(error_str);
	}
}

Lexer::~Lexer() {
	// TODO Auto-generated destructor stub
}

queue<Token> Lexer::getTokens() {
	queue<Token> tokens;
	while ( file.good() ) {
		char c = file.peek();

		if (c == EOF) {
			break;
		}
		else if ( isalpha(c) ) {
			tokens.push( getIdentifier() );
		}
		else if ( isdigit(c) ) {
			tokens.push( getNumerical() );
		}
		else if ( ispunct(c) ) {
			file.get(c);
			tokens.push( Token(string(1, c)) );
		}
		else if ( isspace(c) ) {
			file.get();
		}
		else {
			file.get(c);
			tokens.push( Token(string(1, c)) );
		}

		// TODO numbers

	}

	return tokens;
}

Token Lexer::getIdentifier() {
	string s;
	char c;
	while ( file.good() ) {
		char c = file.peek();
		if ( isalnum(c) || c == '_' ) {
			s += c;
			file.get();
		}
		else {
			break;
		}
	}

	return Token(s);
}

Token Lexer::getNumerical() {
	string s;
	char c;
	while ( file.good() ) {
		char c = file.peek();
		if ( isdigit(c) ) {
			s += c;
			file.get();
		}
		else {
			break;
		}
	}

	return Token(s);
}

void Lexer::skipWhiteSpace() {

}

} /* namespace std */
