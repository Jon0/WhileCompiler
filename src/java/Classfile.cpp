/*
 * Classfile.cpp
 *
 *  Created on: 2/05/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "../lang/Program.h"

#include "Bytecode.h"
#include "Classfile.h"

namespace std {

Classfile::Classfile(){
	cp = make_shared<ConstantPool>();
}

Classfile::Classfile(shared_ptr<Program> p){
	program = p;
	cp = make_shared<ConstantPool>();
}

Classfile::~Classfile() {
	// TODO Auto-generated destructor stub
}

void Classfile::beginFunction( string s ) {
	functions.insert( map<string, shared_ptr<JavaFunction>>::value_type(s, make_shared<JavaFunction>() ) );

	cp->add(make_shared<UTF8>( "main" ));
	cp->add(make_shared<UTF8>("([Ljava/lang/String;)V"));
}

void Classfile::read(string fname) {}

void Classfile::write() {}

string Classfile::classname() {
	return "rubbish";
}

shared_ptr<ConstantPool> Classfile::getConstPool() {
	return cp;
}

vector<unsigned char> Classfile::getBytes() {
	bytes.clear();

	cp->writeByteCode( *shared_from_this() );

	//	u2 access_flags;
	write_u2(1);

	//	u2 this_class;
	write_u2(4);

	//	u2 super_class;
	write_u2(3);

	//	u2 interfaces_count;
	write_u2(0);

	//	u2	interfaces[interfaces_count];

	//	u2 	fields_count;
	write_u2(0);
	//	field_info 	fields[fields_count];

	// Write Methods of this class
	//	u2 	methods_count;
	//	method_info methods[methods_count];
	//FuncMap fm = p->getFuncMap();
	write_u2( functions.size() );

	for ( map<string, shared_ptr<JavaFunction>>::value_type func: functions ) {
		func.second->writeByteCode( shared_from_this() );
	}

	//	u2 	 attributes_count;
	write_u2(0);

	//	attribute_info 	attributes[attributes_count];

	return bytes;
}

void Classfile::write_u4(unsigned int i) {
	bytes.push_back( (unsigned char) (i >> 24) );
	bytes.push_back( (unsigned char) (i >> 16) );
	bytes.push_back( (unsigned char) (i >> 8) );
	bytes.push_back( (unsigned char) (i >> 0) );
}

void Classfile::write_u2(unsigned short i) {
	bytes.push_back( (unsigned char) (i >> 8) );
	bytes.push_back( (unsigned char) (i >> 0) );
}

void Classfile::write_u1(unsigned char i) {
	bytes.push_back( (unsigned char) i );
}

void Classfile::write_str(string i) {
	for (unsigned char c: i) {
		write_u1(c);
	}
}

} /* namespace std */
