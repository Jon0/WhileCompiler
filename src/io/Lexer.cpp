#include <iostream>
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

	current_line = 1;
}

Lexer::~Lexer() {}

queue<Token> Lexer::getTokens() {
	queue<Token> tokens;
	while ( file.good() ) {
		char c = file.peek();

		if (c == EOF) {
			break;
		}
		else if ( c == '/' ) {
			file.get();
			char second = file.peek();
			if (second == '/') {
				file.get();
				skipRestOfLine();
			}
			else if (second == '*') {
				file.get();
				skipRestOfComment();
			}
			else {
				tokens.push( Token(string(1, c), current_line) );
			}
		}
		else if ( isalpha(c) ) {
			tokens.push( getIdentifier() );
		}
		else if ( isdigit(c) ) {
			tokens.push( getNumerical() );
		}
		else if ( c == '"') {
			file.get();
			tokens.push( Token(string(1, c), current_line) );
			tokens.push( getString() );
			file.get(c);
			tokens.push( Token(string(1, c), current_line) );
		}
		else if ( c == '\'') {
			tokens.push( Token(string(1, c), current_line) );
			file.get();
			file.get(c);
			tokens.push( Token(string(1, c), current_line) );
			file.get(c);
			if (c != '\'') {
				throw invalid_argument("char reading failed");
			}
			tokens.push( Token(string(1, c), current_line) );
		}
		else if ( ispunct(c) ) {
			file.get(c);
			tokens.push( Token(string(1, c), current_line) );
		}
		else if ( isspace(c) ) {
			if (c == '\n') current_line++;
			file.get();
		}
		else {
			file.get(c);
			tokens.push( Token(string(1, c), current_line) );
		}
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

	return Token(s, current_line);
}

Token Lexer::getNumerical() {
	string s;
	char c = file.peek();
	while ( file.good() ) {
		char c = file.peek();
		if ( isdigit(c) || c == '.' ) {
			s += c;
			file.get();
		}
		else {
			break;
		}
	}

	return Token(s, current_line);
}

Token Lexer::getString() {
	string s;
	char c;
	while ( file.good() ) {
		char c = file.peek();
		if ( !(c == '"') ) {
			s += c;
			file.get();
		}
		else {
			break;
		}
	}

	return Token(s, current_line);
}

void Lexer::skipWhiteSpace() {

}

void Lexer::skipRestOfLine() {
	while ( file.good() ) {
		char c;
		file.get(c);
		if ( c == '\n' ) {
			break;
		}
	}
}

void Lexer::skipRestOfComment() {
	while ( file.good() ) {
		char c;
		file.get(c);
		if ( c == '*' ) {
			file.get(c);
			if (c == '/') {
				break;
			}
		}
	}
}

} /* namespace std */
