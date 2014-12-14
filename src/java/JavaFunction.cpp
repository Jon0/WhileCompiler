#include <iostream>

#include "Bytecode.h"
#include "Classfile.h"
#include "Constant.h"
#include "ConstantPool.h"
#include "JavaFunction.h"

namespace std {

JavaFunction::JavaFunction( shared_ptr<ConstantPool> pool ) {
	num_locals = 1; // 1 argument to the main method
	name = pool->use( make_shared<UTF8>("main") );
	descriptor = pool->use( make_shared<UTF8>("([Ljava/lang/String;)V") );
	code_string = pool->use( make_shared<UTF8>("Code") );

	cout << "fuction main" << endl;
	cout << "code length = " << codeSize() << endl;
	cout << "max stack = 20" << endl;
	cout << "max locals = " << num_locals << endl;
	cout << endl;
}

JavaFunction::~JavaFunction() {}

unsigned int JavaFunction::codeSize() {
	unsigned int total = 0;
	for (JavaInstruction ji: inst_list) {
		total += ji.size();
	}
	return total;
}

unsigned int JavaFunction::numLocals() {
	return num_locals;
}

void JavaFunction::add(JavaInstruction ji) {
	inst_list.push_back(ji);
}

void JavaFunction::pushConst(short addr) {
	add(JavaInstruction(ldc, make_shared<ConstReference>(addr, 1)));
}

void JavaFunction::branch() {
	add(JavaInstruction(ldc, make_shared<ConstReference>(21, 1)));
	add(JavaInstruction(ldc, make_shared<ConstReference>(21, 1)));
	add(JavaInstruction(ifeq, make_shared<ConstReference>(21-4, 2)));
}

bytecode JavaFunction::writeByteCode() {
	bytecode b;
	write_u2(b, access_public + access_static); // access = public static
	write_u2(b, name);
	write_u2(b, descriptor);


//	if ( f->name() == "main" ) {
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


	//	Code_attribute {
	unsigned int codesize = codeSize();
	write_u2(b, code_string); // "Code"
	write_u4(b, codesize + 12); // size of following block

	//	u2 max_stack;
	write_u2(b, 20); // just guessing

	//	u2 max_locals;
	write_u2(b, num_locals);

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

