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
#include <vector>

namespace io {
namespace parser {
using namespace std;

/**
 * token tracks file origin and the line and char indexes
 */
class Token {
public:
	Token();
	Token(std::string primary_value, std::string file, int line_ind, std::string context, int char_ind);
	virtual ~Token();

	/**
	 * check for a char in the primary token value
	 */
	bool contains(char);

	/**
	 * returns the primary token value
	 */
	string text() const;

	/**
	 * file from which token was read
	 */
	string file() const;

	/**
	 * line in the file the token occured on
	 */
	int lineNum() const;

	/**
	 * the complete line containing the token for context
	 */
	string lineText() const;

	/**
	 * the character index the token begins on from the complete line
	 */
	int charNum() const;

	/**
	 * compares only the primary token value
	 */ 
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

string makeErrorMsg(Token &, string);
string makeErrorMsgA(vector<Token> &, string);

class TokenException: public runtime_error {
public:
	TokenException(Token &t, string s): runtime_error(makeErrorMsg(t, s)) {}
	TokenException(vector<Token> &t, string s): runtime_error(makeErrorMsgA(t, s)) {}
};

} /* namespace parser */
} /* namespace io */

#endif /* TOKEN_H_ */
