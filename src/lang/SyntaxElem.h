#ifndef SYNTAX_H_
#define SYNTAX_H_

#include <memory>
#include <vector>

#include "../io/Token.h"

#include "SyntaxVisitor.h"

namespace lang {

class Value;

/**
 * a parsed syntax element, and the corresponding tokens
 * read from the source file which are used to report errors
 */
class SyntaxElem {
public:
	SyntaxElem() {}
	virtual ~SyntaxElem() {}

	/**
	 * add token used in produced error messages
	 */
	void addToken(io::parser::Token t) {
		tok.push_back(t);
	}

	std::vector<io::parser::Token> &getTokens() {
		return tok;
	}

	void copyTokens(SyntaxElem &other) {
		tok.insert(tok.end(), other.tok.begin(), other.tok.end());
	}

	int tokenSize() {
		return tok.size();
	}

	/* */
	virtual void visit(std::shared_ptr<SyntaxVisitor> v) = 0;

	void visitChildren(std::shared_ptr<SyntaxVisitor> v) {
		for (std::shared_ptr<SyntaxElem> e: children) {
			e->visit(v);
		}
	}

protected:
	void addChild(std::shared_ptr<SyntaxElem> e) {
		children.push_back(e);
	}

private:
	std::vector<io::parser::Token> tok;

	std::vector<std::shared_ptr<SyntaxElem>> children;
};

} /* namespace lang */

#endif /* SYNTAX_H_ */
