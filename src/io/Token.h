/*
 * Token.h
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#ifndef TOKEN_H_
#define TOKEN_H_

#include <functional>
#include <string>

namespace std {



class Token {
public:
	Token();
	Token(string, string, int, string, int);
	virtual ~Token();

	bool contains(char);
	string text() const;
	string file() const;
	int lineNum() const;
	string lineText() const;
	int charNum() const;

	bool operator==(const Token &other) const;

private:
	string input_text;
	string filename;
	int line_num;
	string line_copy;
	int char_num;
};

struct TokenHash {
   size_t operator() (const Token &t) const {
     return (t.text().length());
   }
};

struct TokenCompare {
	bool operator()( const Token &first , const Token &second) const {
		return first.text().compare(second.text()) < 0? true: false;
	}
};

string makeErrorMsg(Token, string);

class TokenException: public runtime_error {
public:
	TokenException(Token t, string s): runtime_error(makeErrorMsg(t, s)) {}
};

} /* namespace std */

#endif /* TOKEN_H_ */
