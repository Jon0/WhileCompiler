/*
 * Bytecode.cpp
 *
 *  Created on: 19/04/2014
 *      Author: asdf
 */

#include <iostream>

#include "../lang/Program.h"
#include "../lang/Expr.h"
#include "../lang/Func.h"
#include "../lang/Stmt.h"

#include "Bytecode.h"

namespace std {

Bytecode::Bytecode(ClassfileWriter &o, ConstantPool &v): out(o), constant_pool(v) {}

Bytecode::~Bytecode() {
	// TODO Auto-generated destructor stub
}

int Bytecode::stackSize() {
	int total = 0;
	for (Instruction &i: istack) {
		total += i.bytes.size();
	}
	return total;
}

void Bytecode::addInstruction(unsigned char code) {
	Instruction i;
	i.bytes.push_back(code);
	istack.push_back(i);
}

void Bytecode::addInstruction1(unsigned char code, unsigned char arg) {
	Instruction i;
	i.bytes.push_back(code);
	i.bytes.push_back(arg);
	istack.push_back(i);
}

void Bytecode::addInstruction2(unsigned char code, unsigned short arg) {
	Instruction i;
	i.bytes.push_back(code);
	i.bytes.push_back((unsigned char) (arg >> 8));
	i.bytes.push_back((unsigned char) (arg >> 0));
	istack.push_back(i);
}

void Bytecode::addInstruction4(unsigned char code, unsigned int arg) {
	Instruction i;
	i.bytes.push_back(code);
	i.bytes.push_back((unsigned char) (arg >> 24));
	i.bytes.push_back((unsigned char) (arg >> 16));
	i.bytes.push_back((unsigned char) (arg >> 8));
	i.bytes.push_back((unsigned char) (arg >> 0));
	istack.push_back(i);
}

void Bytecode::accept(shared_ptr<Type>) {}
void Bytecode::accept(shared_ptr<Value>) {}

/* create flow for each function */
void Bytecode::accept(shared_ptr<Program> p) {

	//	u2 access_flags;
	out.write_u2(1);

	//	u2 this_class;
	out.write_u2(4);

	//	u2 super_class;
	out.write_u2(3);

	//	u2 interfaces_count;
	out.write_u2(0);

//	u2	interfaces[interfaces_count];

//	u2 	fields_count;
	out.write_u2(0);
//	field_info 	fields[fields_count];


	// Write Methods of this class
	//	u2 	methods_count;
	out.write_u2(1);


	//	method_info methods[methods_count];
	FuncMap fm = p->getFuncMap();

	for ( FuncMap::value_type func: fm ) {
		func.second->visit(shared_from_this());
	}


	//	u2 	 attributes_count;
	out.write_u2(0);

	//	attribute_info 	attributes[attributes_count];
}

void Bytecode::accept(shared_ptr<Func> f) {
	cout << "write function " << f->name() << endl;


	out.write_u2(9); // access


	out.write_u2( constant_pool.lookup("main") );

	out.write_u2(6); // descriptor
	out.write_u2(1); // number of attributes



	// visit function code
	f->getStmt()->visit(shared_from_this());
	addInstruction(0xb1); // add return


	int codesize = stackSize();
	cout << "code length = " << codesize << endl;

	//	Code_attribute {
	out.write_u2( constant_pool.lookup("Code") ); // "Code"
	out.write_u4( codesize + 12 ); // size of following block

	//	u2 max_stack;
	out.write_u2(2); // just guessing

	//	u2 max_locals;
	out.write_u2(1); // 1 argument, no variables

	//	u4 code_length;
	out.write_u4( codesize );
	 // return
	//	u1 code[code_length];
	for (Instruction &i: istack) {
		for (char b: i.bytes) {
			out.write_u1(b);
		}
	}

	istack.clear();

	//	u2 exception_table_length;
	out.write_u2(0);

//	{
//	u2 start_pc;
//	u2 end_pc;
//	u2 handler_pc;
//	u2 catch_type;
//	} exception_table[exception_table_length];


//	u2 attributes_count;
	out.write_u2(0);

//	attribute_info attributes[attributes_count];
//	}
}

void Bytecode::accept(shared_ptr<BlockStmt> bs) {
	for (shared_ptr<Stmt> s: bs->getStmt()) {
		s->visit(shared_from_this());
	}
}

void Bytecode::accept(shared_ptr<InitStmt>) {}
void Bytecode::accept(shared_ptr<AssignStmt>) {}
void Bytecode::accept(shared_ptr<IfStmt>) {}
void Bytecode::accept(shared_ptr<WhileStmt>) {}
void Bytecode::accept(shared_ptr<ForStmt>) {}

void Bytecode::accept(shared_ptr<PrintStmt> ps) {
	addInstruction2(0xb2, 13); // push output stream

	ps->getExpr()->visit(shared_from_this()); // should add item to print

	addInstruction2(0xb6, 19); // invoke println
}

void Bytecode::accept(shared_ptr<EvalStmt>) {}
void Bytecode::accept(shared_ptr<ReturnStmt>) {}
void Bytecode::accept(shared_ptr<BreakStmt>) {}
void Bytecode::accept(shared_ptr<SwitchStmt>) {}

void Bytecode::accept(shared_ptr<ConstExpr> ex) {
	string s = ex->getValue()->asString();

	addInstruction1(0x12, constant_pool.lookupType("string", s)); // push constant string
}


void Bytecode::accept(shared_ptr<IsTypeExpr>) {}
void Bytecode::accept(shared_ptr<VariableExpr>) {}
void Bytecode::accept(shared_ptr<FuncCallExpr>) {}
void Bytecode::accept(shared_ptr<RecordExpr>) {}

void Bytecode::accept(shared_ptr<ListExpr>) {}
void Bytecode::accept(shared_ptr<ListLengthExpr>) {}
void Bytecode::accept(shared_ptr<ConcatExpr>) {}
void Bytecode::accept(shared_ptr<ListLookupExpr>) {}
void Bytecode::accept(shared_ptr<RecordMemberExpr>) {}
void Bytecode::accept(shared_ptr<BasicCastExpr>) {}
void Bytecode::accept(shared_ptr<AbstractOpExpr>) {}

} /* namespace std */
