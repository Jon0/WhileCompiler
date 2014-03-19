/*
 * Var.cpp
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#include "Var.h"

namespace std {

Var::Var() {

}

Var::Var(shared_ptr<Type> t, string n): type_info(t) {
	name_str = n;
}

Var::~Var() {
	// TODO Auto-generated destructor stub
}

string Var::name() const {
	return name_str;
}

shared_ptr<Type> Var::type() const {
	return type_info;
}

} /* namespace std */
