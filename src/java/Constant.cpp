/*
 * Constant.cpp
 *
 *  Created on: 23/06/2014
 *      Author: asdf
 */

#include "Bytecode.h"
#include "Constant.h"

namespace std {

Constant::Constant(const_type t) {
	ind = -1;
	type = t;
}

const_type Constant::getType() const {
	return type;
}

bool Constant::equalChildren( const_list l ) const {
	if (children.size() != l.size()) return false;
	for (int i = 0; i < l.size(); ++i) {
		if (*children[i] != *l[i]) return false;
	}
	return true;
}

bool Constant::operator!=( const Constant &other ) const {
	return !(*this == other);
}

bool Constant::operator==(const Constant &other) const {
	// the type and name are equal
	if (other.getType() == getType()) {
		return equalChildren(other.getChildren());
	}
	return false;
}

bytecode Constant::writeByteCode() const {
	bytecode b;  //empty list
	return b;
}

bool UTF8::operator==( const Constant &other ) const {

	if (other.getType() == getType()) {
		UTF8 &u = (UTF8 &)other;
		return u.str == str;
	}

	return false;
}

bytecode UTF8::writeByteCode() const {
	bytecode b;
	write_u2(b, str.length());
	write_str(b, str);
	return b;
}

string UTF8::lookupStr() {
	return str;
}

string UTF8::typeStr() {
	return "utf8";
}

string JClass::lookupStr() {
	return "class";
}

string JClass::typeStr() {
	return "class";
}

string JNameType::lookupStr() {
	return "nd";
}

string JNameType::typeStr() {
	return "nametype";
}

string JFieldRef::lookupStr() {
	return "fr";
}

string JFieldRef::typeStr() {
	return "fieldref";
}

string JMethodRef::lookupStr() {
	return "mr";
}

string JMethodRef::typeStr() {
	return "methodref";
}

bool JString::operator==( const Constant &other ) const {
	// the type and name are equal
	if (other.getType() == getType()) {
		JString &u = (JString &)other;
		return equalChildren( u.getChildren() );
	}
	return false;
}

bytecode JString::writeByteCode() const {
	bytecode b;
	return b;
}

string JString::lookupStr() {
	return "str";
}

string JString::typeStr() {
	return "string";
}

bool JInteger::operator==( const Constant &other ) const {
	return true;
}

bytecode JInteger::writeByteCode() const {
	//outfile.write_u1(3); // done outside now
	bytecode b;
	write_u4(b, value);
	return b;
}

string JInteger::lookupStr() {
	return "str";
}

string JInteger::typeStr() {
	return "integer";
}

int JInteger::getValue() {
	return value;
}

bool JFloat::operator==( const Constant &other ) const {
	return true;
}

bytecode JFloat::writeByteCode() const {
	//outfile.write_u1(4); // done outside now
	bytecode b;
	write_u4(b, value);
	return b;
}

string JFloat::lookupStr() {
	return "str";
}

string JFloat::typeStr() {
	return "float";
}

} /* namespace std */
