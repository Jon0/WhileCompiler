/*
 * Token.cpp
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#include "Token.h"

namespace std {

Token::Token(string s, int l) {
	input_text = s;
	line_num = l;
}

Token::~Token() {
	// TODO Auto-generated destructor stub
}

bool Token::contains(char c) {
	for (char cs: input_text) {
		if (cs == c) return true;
	}
	return false;
}

string Token::text() const {
	return input_text;
}

int Token::lineNum() const {
	return line_num;
}

bool Token::operator==(const Token &other) const {
	return (input_text == other.input_text);
}

} /* namespace std */
