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
	Token(string, int);
	virtual ~Token();

	bool contains(char);
	string text() const;
	int lineNum() const;
	bool operator==(const Token &other) const;

private:
	string input_text;
	int line_num;
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

class TokenException: public runtime_error {
public:
	TokenException(Token t, string s): runtime_error("Line "+to_string(t.lineNum())+": "+s) {}
};

} /* namespace std */

#endif /* TOKEN_H_ */
