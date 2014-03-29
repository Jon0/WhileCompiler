#ifndef PARSERATTR_H_
#define PARSERATTR_H_

#include <map>
#include <unordered_set>
#include <vector>

#include "Lexer.h"
#include "../lang/Var.h"

namespace std {

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

	Token match(string s) {
		Token top = tokens.front();
		if ( s != top.text() ) {
			throw TokenException(top, "mismatch "+top.text()+", expected "+s);
		}
		return pop();
	}

	bool canMatch(string s) {
		string top = tokens.front().text();
		if ( s == top ) {
			tokens.pop();
			return true;
		}
		return false;
	}


private:
	queue<Token> tokens;
};

class ParserContext {
public:
	bool isVar(Token t) {
		return vars.count(t) > 0;
	}

	Var copyVar(Token t) {
		if (var_types.count(t) == 0) {
			throw TokenException(t, t.text()+" not a variable");
		}
		return var_types[t];
	}

	shared_ptr<Type> getType(Token t) {
		if (var_types.count(t) == 0) {
			throw TokenException(t, t.text()+" not a variable");
		}
		return var_types[t].type();
	}

	void initialise(shared_ptr<Type> t, Token name) {
		vars.insert(name);
		var_types[name] = Var(t, name);
	}

private:
	unordered_set<Token, TokenHash> vars;
	map<Token, Var, TokenCompare> var_types;
};

} /* namespace std */

#endif /* PARSERATTR_H_ */
