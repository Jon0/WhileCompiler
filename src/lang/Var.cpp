/*
 * Var.cpp
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#include "Var.h"

namespace std {

Var::Var() {}

Var::Var(shared_ptr<Type> t, Token n): type_info(t), in_token(n) {}

Var::Var(shared_ptr<Type> yt, Var v): type_info(yt), in_token(v.in_token) {}

Var::~Var() {}

string Var::name() const {
	return in_token.text();
}

Token Var::token() const {
	return in_token;
}

shared_ptr<Type> Var::type() const {
	return type_info;
}

} /* namespace std */
