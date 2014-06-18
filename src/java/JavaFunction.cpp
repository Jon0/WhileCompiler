/*
 * JavaFunction.cpp
 *
 *  Created on: 19/06/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "Classfile.h"
#include "JavaFunction.h"

namespace std {

JavaFunction::JavaFunction() {
	// TODO Auto-generated constructor stub

}

JavaFunction::~JavaFunction() {
	// TODO Auto-generated destructor stub
}

void JavaFunction::writeByteCode( shared_ptr<Classfile> out ) {
	out->write_u2(9); // access = public static

	out->write_u2( out->getConstPool()->lookup( "main" ) );

//	if ( f->name() == "main" ) {
	out->write_u2( out->getConstPool()->lookup("([Ljava/lang/String;)V") ); // descriptor

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
	out->write_u2(1); // number of attributes - currently only code

	// visit function code
	//f->getStmt()->visit(shared_from_this());
	//if ( f->returnType()->nameStr() == "void") {
	//	addInstruction(0xb1); // add return
	//}

	int codesize = 1; //stackSize();
	cout << "code length = " << codesize << endl;

	//	Code_attribute {
	out->write_u2( out->getConstPool()->lookup("Code") ); // "Code"
	out->write_u4( codesize + 12 ); // size of following block

	//	u2 max_stack;
	out->write_u2(20); // just guessing
	cout << "max stack = 20" << endl;

	//	u2 max_locals;
	out->write_u2(num_locals);
	cout << "max locals = " << num_locals << endl;

	//	u4 code_length;
	out->write_u4( codesize );
	 // return
	//	u1 code[code_length];
//	for (Instruction &i: istack) {
//		for (char b: i.bytes) {
//			out->write_u1(b);
//		}
//	}
	out->write_u1(0xb1); // ret

//	istack.clear();
//	local_map.clear();
//	local_type.clear();

	//	u2 exception_table_length;
	out->write_u2(0);

//	{
//	u2 start_pc;
//	u2 end_pc;
//	u2 handler_pc;
//	u2 catch_type;
//	} exception_table[exception_table_length];


//	u2 attributes_count;
	out->write_u2(0);

//	attribute_info attributes[attributes_count];
//	}
}

} /* namespace std */

