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


	//	add(make_shared<UTF8>("java/lang/Object"));		// #1
	//	add(make_shared<UTF8>("rubbish"));				// #2
	//	add(make_shared<JClass>(1));					// #3
	//	add(make_shared<JClass>(2));					// #4

	thisclass = cp->use( make_shared<JClass>("rubbish") );
	superclass = cp->use( make_shared<JClass>("java/lang/Object") );

}

// no longer used....
Classfile::Classfile(shared_ptr<Program> p){
	program = p;
	cp = make_shared<ConstantPool>();
	thisclass = 0;
	superclass = 0;
}

Classfile::~Classfile() {
	// TODO Auto-generated destructor stub
}

void Classfile::beginFunction( string s ) {
	functions.insert( map<string, shared_ptr<JavaFunction>>::value_type(s, make_shared<JavaFunction>() ) );

	cp->add(make_shared<UTF8>( "main" ));
	cp->add(make_shared<UTF8>("([Ljava/lang/String;)V"));
	cp->add(make_shared<UTF8>( "Code" ));
}

void Classfile::call() {

	//	// //FieldRef(Class(Utf8("java/lang/System")), NameType(Utf8("out"), Utf8("Ljava/io/PrintStream;")))
	//	add(make_shared<UTF8>("java/lang/System")); 	// #6
	//	add(make_shared<JClass>(6));					// #7
	//	add(make_shared<UTF8>("out"));						// #8
	//	add(make_shared<UTF8>("Ljava/io/PrintStream;"));	// #9
	//	add(make_shared<JNameType>(8, 9));				// #10
	//	add(make_shared<JFieldRef>(7, 10));					// #11


	// push output stream
	short addr = cp->use( make_shared<JFieldRef>("java/lang/System", "out", "Ljava/io/PrintStream;") );
	shared_ptr<JavaReference> ref = make_shared<ConstReference>(addr, 2);
	functions["main"]->addInstruction(JavaInstruction(getstatic, IArgs{ref}));

	// push value to print
	addr = cp->use(make_shared<JString>( "Hello" ));
	ref = make_shared<ConstReference>(addr, 1);
	functions["main"]->addInstruction(JavaInstruction(ldc, IArgs{ref}));

	// invoke println for str
	addr = cp->use(make_shared<JMethodRef>( "java/io/PrintStream", "println", "(Ljava/lang/String;)V" ));
	ref = make_shared<ConstReference>(addr, 2);
	functions["main"]->addInstruction(JavaInstruction(invoke, IArgs{ref}));

	functions["main"]->addInstruction(JavaInstruction(ret, {}));
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
		write_list( bytes, func.second->writeByteCode( shared_from_this() ) );
	}

	//	u2 	 attributes_count;
	write_u2(bytes, 0);

	//	attribute_info 	attributes[attributes_count];

	return bytes;
}

} /* namespace std */
