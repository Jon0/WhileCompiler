#ifndef PARSERATTR_H_
#define PARSERATTR_H_

#include <map>
#include <unordered_set>
#include <vector>

#include "Lexer.h"
#include "../lang/Var.h"

namespace io {
namespace parser {

/**
 * matches tokens from a stream of tokens
 */
class ParserInput {
public:
	ParserInput(Lexer &lexer) {
		tokens = lexer.getTokens();
	}


	bool empty() {
		return tokens.empty();
	}

	Token peek() {
		return tokens.front();
	}

	Token pop() {
		Token t = tokens.front();
		tokens.pop();
		return t;
	}

	Token match(std::string s) {
		Token top = tokens.front();
		if ( s != top.text() ) {
			throw TokenException(top, "mismatch "+top.text()+", expected "+s);
		}
		return pop();
	}

	bool canMatch(std::string s) {
		std::string top = tokens.front().text();
		if ( s == top ) {
			tokens.pop();
			return true;
		}
		return false;
	}


private:
	std::queue<Token> tokens;
};

/**
 * determines if a token is a known variable in the current scope
 */
class ParserContext {
public:
	bool isVar(Token t) {
		return vars.count(t) > 0;
	}

	/**
	 * use token to find a name and type pair
	 */
	lang::Var copyVar(Token t) {
		if (var_types.count(t) == 0) {
			throw TokenException(t, t.text()+" not a variable");
		}
		return var_types[t];
	}

	/**
	 * gets only the type of a known variable
	 */
	std::shared_ptr<lang::Type> getType(Token t) {
		if (var_types.count(t) == 0) {
			throw TokenException(t, t.text()+" not a variable");
		}
		return var_types[t].type();
	}

	/**
	 * add a new variable to the current context
	 */
	void initialise(std::shared_ptr<lang::Type> t, Token name) {
		vars.insert(name);
		var_types[name] = lang::Var(t, name);
	}

private:
	std::unordered_set<Token, TokenHash> vars;
	std::map<Token, lang::Var, TokenCompare> var_types;
};

} /* namespace parser */
} /* namespace io */

#endif /* PARSERATTR_H_ */
