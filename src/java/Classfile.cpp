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
#include "Constant.h"

namespace std {

Classfile::Classfile(){
	cp = make_shared<ConstantPool>();
	thisclass = cp->use( make_shared<JClass>("rubbish") );
	superclass = cp->use( make_shared<JClass>("java/lang/Object") );
}

Classfile::~Classfile() {}

void Classfile::beginFunction( string s ) {
	functions.insert( map<string, shared_ptr<JavaFunction>>::value_type(s, make_shared<JavaFunction>(cp) ) );
}

void Classfile::call() {
	// push output stream
	short addr = cp->use( make_shared<JFieldRef>("java/lang/System", "out", "Ljava/io/PrintStream;") );
	functions["main"]->add(JavaInstruction(getstatic, make_shared<ConstReference>(addr, 2)));

	// push value to print
	//addr = cp->use(make_shared<JString>( "Hello" ));
	addr = cp->use(make_shared<JInteger>( 12 ));
	functions["main"]->add(JavaInstruction(ldc, make_shared<ConstReference>(addr, 1)));

	addr = cp->use(make_shared<JInteger>( 5 ));
	functions["main"]->add(JavaInstruction(ldc, make_shared<ConstReference>(addr, 1)));

	functions["main"]->add(JavaInstruction(iadd));

	addr = cp->use(make_shared<JInteger>( 1 ));
	functions["main"]->add(JavaInstruction(ldc, make_shared<ConstReference>(addr, 1)));

	functions["main"]->add(JavaInstruction(iadd));

	// invoke println for str
	//addr = cp->use(make_shared<JMethodRef>( "java/io/PrintStream", "println", "(Ljava/lang/String;)V" ));
	addr = cp->use(make_shared<JMethodRef>( "java/io/PrintStream", "println", "(I)V" ));
	functions["main"]->add(JavaInstruction(invoke, make_shared<ConstReference>(addr, 2)));

	functions["main"]->add(JavaInstruction(ret));
}

string Classfile::classname() {
	return "rubbish";
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
