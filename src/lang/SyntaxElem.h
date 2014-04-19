#ifndef SYNTAX_H_
#define SYNTAX_H_

#include <memory>
#include <vector>

#include "../io/Token.h"

#include "SyntaxVisitor.h"

namespace std {

class Value;

class SyntaxElem {
public:
	SyntaxElem() {}
	virtual ~SyntaxElem() {}

	void addToken(Token t) {
		tok.push_back(t);
	}

	vector<Token> &getTokens() {
		return tok;
	}

	void copyTokens(SyntaxElem &other) {
		tok.insert(tok.end(), other.tok.begin(), other.tok.end());
	}

	int tokenSize() {
		return tok.size();
	}

	/* */
	virtual void visit(shared_ptr<SyntaxVisitor> v) = 0;

private:
	vector<Token> tok;
};

} /* namespace std */

#endif /* SYNTAX_H_ */
