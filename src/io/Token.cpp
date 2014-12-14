/*
 * Token.cpp
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "Token.h"

namespace io {
namespace parser {
using namespace std;

Token::Token() {}

Token::Token(string primary_value, string file, int line_ind, string context, int char_ind)
	:
	input_text {primary_value},
	filename {file},
	line_num {line_ind},
	line_copy {context},
	char_num {char_ind} {
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

string makeErrorMsg(Token &t, string s) {
	string result;
	result += t.file();
	result += ":";
	result += to_string(t.lineNum());
	result += ": ";
	result += s;
	result += "\n";
	result += t.lineText();
	result += "\n";

	int spos = t.charNum() - t.text().length();
	int epos = t.charNum();
	for (int i = 0; i < spos; ++i) result += " ";
	for (int i = spos; i < epos; ++i) result += "^";

	return result;
}

string makeErrorMsgA(vector<Token> &toks, string s) {
	if (toks.size() == 0) {
		return "cannot find tokens";
	}
	Token &t = toks[0];
	string result;
	result += t.file();
	result += ":";
	result += to_string(t.lineNum());
	result += ": ";
	result += s;
	result += "\n";
	result += t.lineText();
	result += "\n";

	int spos = t.charNum() - t.text().length();
	int epos = t.charNum();
	for (Token &tt: toks) {
		if (tt.charNum() - tt.text().length() < spos) spos = tt.charNum() - tt.text().length();
		if (tt.charNum() > epos) epos = tt.charNum();
	}
	for (int i = 0; i < spos; ++i) result += " ";
	for (int i = spos; i < epos; ++i) result += "^";

	return result;
}

} /* namespace parser */
} /* namespace io */
