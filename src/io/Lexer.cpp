#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include "Lexer.h"

namespace std {

Lexer::Lexer(const char *fn):
		file(fn)
{
	vector<string> elems;
	stringstream ss(fn);
	string item;
	while (getline(ss, item, '/')) {
		elems.push_back(item);
	}
	filename = elems[elems.size()-1];


	if (!file) {
		string error_str = string("cannot find ").append(filename);
		throw invalid_argument(error_str);
	}

	current_line = 1;
	getline(file, current_line_str);
	current_line_char = 0;
}

Lexer::~Lexer() {}

queue<Token> Lexer::getTokens() {
	queue<Token> tokens;
	while ( canRead() ) {
		char c = peekChar();

		if (c == EOF) {
			break;
		}
		else if ( c == '/' ) {
			popChar();
			char second = peekChar();
			if (second == '/') {
				incLine();
			}
			else if (second == '*') {
				popChar();
				skipRestOfComment();
			}
			else {
				tokens.push( makeToken(string(1, c)) );
			}
		}
		else if ( isalpha(c) ) {
			tokens.push( getIdentifier() );
		}
		else if ( isdigit(c) ) {
			tokens.push( getNumerical() );
		}
		else if ( c == '"') {
			popChar();
			tokens.push( makeToken(string(1, c)) );
			tokens.push( getString() );
			c = popChar();
			tokens.push( makeToken(string(1, c)) );
		}
		else if ( c == '\'') {
			popChar();
			tokens.push( makeToken(string(1, c)) );
			c = popChar();
			tokens.push( makeToken(string(1, c)) );
			c = popChar();
			if (c != '\'') {
				throw invalid_argument("lexer: char reading failed");
			}
			tokens.push( makeToken(string(1, c)) );
		}
		else if ( ispunct(c) ) {
			c = popChar();
			char n = peekChar();
			if (c == '+' && n == '+') {
				popChar();
				string s = "++";
				tokens.push( makeToken(s) );
			}
			else {
				tokens.push( makeToken(string(1, c)) );
			}
		}
		else if ( isspace(c) ) {
			popChar();
		}
		else {
			c = popChar();
			tokens.push( makeToken(string(1, c)) );
		}
	}

	return tokens;
}

bool Lexer::canRead() {
	return file.good() || current_line_char < current_line_str.length();
}

char Lexer::popChar() {
	char r = peekChar();

	current_line_char++;
	if (current_line_char > current_line_str.length() && file.good()) {
		incLine();
	}
	return r;
}

char Lexer::peekChar() {
	char r;
	if (current_line_char == current_line_str.length()) {
		r = '\n';
	}
	else {
		r = current_line_str[current_line_char];
	}
	return r;
}

Token Lexer::makeToken(string s) {
	return Token(s, filename, current_line, current_line_str, current_line_char);
}

void Lexer::incLine() {
	getline(file, current_line_str);
	current_line_char = 0;
	current_line++;
}

Token Lexer::getIdentifier() {
	string s;
	while ( canRead() ) {
		char c = peekChar();
		if ( isalnum(c) || c == '_' ) {
			s += c;
			popChar();
		}
		else {
			break;
		}
	}

	return makeToken(s);
}

Token Lexer::getNumerical() {
	string s;
	while ( canRead() ) {
		char c = peekChar();
		if ( isdigit(c) || c == '.' ) {
			s += c;
			popChar();
		}
		else {
			break;
		}
	}

	return makeToken(s);
}

Token Lexer::getString() {
	string s;
	while ( canRead() ) {
		char c = peekChar();
		if ( !(c == '"') ) {
			s += c;
			popChar();
		}
		else {
			break;
		}
	}

	return makeToken(s);
}

void Lexer::skipRestOfComment() {
	while ( canRead() ) {
		char c = popChar();
		if ( c == '*' ) {
			c = popChar();
			if (c == '/') {
				break;
			}
		}
	}
}

} /* namespace std */
