#include "Var.h"

namespace lang {
using namespace std;

Var::Var() {}

Var::Var(shared_ptr<Type> type, io::parser::Token name): type_info(type), in_token(name) {}

Var::Var(shared_ptr<Type> type, Var name): type_info(type), in_token(name.in_token) {}

Var::~Var() {}

string Var::name() const {
	return in_token.text();
}

io::parser::Token Var::token() const {
	return in_token;
}

shared_ptr<Type> Var::type() const {
	return type_info;
}

} /* namespace std */
