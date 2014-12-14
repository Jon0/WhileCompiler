#include <iostream>

#include "../lang/Program.h"

#include "Bytecode.h"
#include "Classfile.h"
#include "Constant.h"
#include "JavaObject.h"

namespace std {

Classfile::Classfile(){}

Classfile::~Classfile() {}

void Classfile::initialise( string class_name ) {
	class_name_str = class_name;
	cp = make_shared<ConstantPool>();
	thisclass = cp->use( make_shared<JClass>(class_name_str) );
	superclass = cp->use( make_shared<JClass>("java/lang/Object") );
}

void Classfile::beginFunction( string func_name ) {
	functions.insert( map<string, shared_ptr<JavaFunction>>::value_type(func_name, make_shared<JavaFunction>(cp) ) );
}

void Classfile::call() {
	shared_ptr<JavaFunction> func = functions["main"];

	func->branch();

	shared_ptr<JavaNumber> n1 = make_shared<JavaNumber>(func, cp, 1);
	shared_ptr<JavaNumber> n5 = make_shared<JavaNumber>(func, cp, 5);
	shared_ptr<JavaNumber> n12 = make_shared<JavaNumber>(func, cp, 12);

	// print addition
	n1->add(n5)->add(n12)->print();

	shared_ptr<JavaString> str = make_shared<JavaString>(func, cp);
	str->print();

	func->add(JavaInstruction(ret));
}

string Classfile::classname() {
	return class_name_str;
}

shared_ptr<ConstantPool> Classfile::getConstPool() {
	return cp;
}

vector<unsigned char> Classfile::getBytes() {
	bytes.clear();

	write_list(bytes, cp->getByteCode());

	//	u2 access_flags;
	write_u2(bytes, access_public); // public

	//	u2 this_class;
	write_u2(bytes, thisclass);

	//	u2 super_class;
	write_u2(bytes, superclass);

	//	u2 interfaces_count;
	write_u2(bytes, 0);

	//	u2	interfaces[interfaces_count];

	//	u2 	fields_count;
	write_u2(bytes, 0);
	//	field_info 	fields[fields_count];

	// Write Methods of this class
	//	u2 	methods_count;
	//	method_info methods[methods_count];
	//FuncMap fm = p->getFuncMap();
	write_u2(bytes, functions.size());

	for ( map<string, shared_ptr<JavaFunction>>::value_type func: functions ) {
		write_list( bytes, func.second->writeByteCode() );
	}

	//	u2 	 attributes_count;
	write_u2(bytes, 0);

	//	attribute_info 	attributes[attributes_count];

	return bytes;
}

} /* namespace std */
