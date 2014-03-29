/*
 * Token.cpp
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#include "Token.h"

namespace std {

Token::Token() {}

Token::Token(string s, string fn, int l, string lc, int cn) {
	input_text = s;
	filename = fn;
	line_num = l;
	line_copy = lc;
	char_num = cn;
}

Token::~Token() {}

bool Token::contains(char c) {
	for (char cs: input_text) {
		if (cs == c) return true;
	}
	return false;
}

string Token::text() const {
	return input_text;
}

string Token::file() const {
	return filename;
}

int Token::lineNum() const {
	return line_num;
}

string Token::lineText() const {
	return line_copy;
}

int Token::charNum() const {
	return char_num;
}

bool Token::operator==(const Token &other) const {
	return (input_text == other.input_text);
}

string makeErrorMsg(Token t, string s) {
	string result;
	result += t.file();
	result += ":";
	result += to_string(t.lineNum());
	result += ": ";
	result += s;
	result += "\n";
	result += t.lineText();
	result += "\n";

	int pos = t.charNum() - 1;
	for (int i = 0; i < pos; ++i) result += " ";
	result += "^";

	return result;
}

} /* namespace std */
