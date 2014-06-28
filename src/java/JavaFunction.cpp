/*
 * JavaFunction.cpp
 *
 *  Created on: 19/06/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "Bytecode.h"
#include "Classfile.h"
#include "JavaFunction.h"

namespace std {

JavaFunction::JavaFunction() {}

JavaFunction::~JavaFunction() {}

unsigned int JavaFunction::codeSize() {
	unsigned int total = 0;
	for (JavaInstruction ji: inst_list) {
		total += ji.size();
	}
	return total;
}

void JavaFunction::addInstruction(JavaInstruction ji) {
	inst_list.push_back(ji);
}

bytecode JavaFunction::writeByteCode( shared_ptr<Classfile> out ) {
	bytecode b;

	write_u2(b, 9); // access = public static

	write_u2(b, out->getConstPool()->lookup( "main" ));

//	if ( f->name() == "main" ) {
	write_u2(b, out->getConstPool()->lookup("([Ljava/lang/String;)V")); // descriptor

	int num_locals = 1; // 1 argument to the main method
	//local_type.push_back( NULL ); // reserve for unused argument

//	}
//	else {
//		string desc = JavaDescriptor( f );
//		out.write_u2( constant_pool.lookup(desc) ); // descriptor
//
//		num_locals = 0;
//		vector<Var> args = f->getArgs();
//		for (Var &v: args) {
//			cout << "arg " << v.name() << " is " <<  v.type()->nameStr() << endl;
//
//			local_map.insert( map<string, int>::value_type(v.name(), num_locals) );
//			local_type.push_back( v.type() );
//
//			num_locals += 1;
//		}
//	}
	write_u2(b, 1); // number of attributes - currently only code

	// visit function code
	//f->getStmt()->visit(shared_from_this());
	//if ( f->returnType()->nameStr() == "void") {
	//	addInstruction(0xb1); // add return
	//}

	unsigned int codesize = codeSize();
	cout << "code length = " << codesize << endl;

	//	Code_attribute {
	write_u2(b, out->getConstPool()->lookup("Code")); // "Code"
	write_u4(b, codesize + 12); // size of following block

	//	u2 max_stack;
	write_u2(b, 20); // just guessing
	cout << "max stack = 20" << endl;

	//	u2 max_locals;
	write_u2(b, num_locals);
	cout << "max locals = " << num_locals << endl;

	//	u4 code_length;
	write_u4(b, codesize);
	for (JavaInstruction ji: inst_list) {
		write_list(b, ji.toBytecode());
	}

	//	u2 exception_table_length;
	write_u2(b, 0);

//	{
//	u2 start_pc;
//	u2 end_pc;
//	u2 handler_pc;
//	u2 catch_type;
//	} exception_table[exception_table_length];


//	u2 attributes_count;
	write_u2(b, 0);

//	attribute_info attributes[attributes_count];
//	}
	return b;
}

} /* namespace std */

