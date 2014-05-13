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



	//	method_info methods[methods_count];
	FuncMap fm = p->getFuncMap();
	out.write_u2( fm.size() );


	for ( FuncMap::value_type func: fm ) {
		func.second->visit(shared_from_this());
	}


	//	u2 	 attributes_count;
	out.write_u2(0);

	//	attribute_info 	attributes[attributes_count];
}

void Bytecode::accept(shared_ptr<Func> f) {
	cout << "write function " << f->name() << endl;

	out.write_u2(9); // access = public static
	out.write_u2( constant_pool.lookup( f->name() ) );

	if ( f->name() == "main" ) {
		out.write_u2( constant_pool.lookup("([Ljava/lang/String;)V") ); // descriptor

		num_locals = 1; // 1 argument to the main method
		local_type.push_back( NULL ); // reserve for unused argument

	}
	else {
		out.write_u2( constant_pool.lookup("(I)V") ); // descriptor

		num_locals = 0;
		vector<Var> args = f->getArgs();
		for (Var &v: args) {

			cout << "arg " << v.name() << " is " <<  v.type()->nameStr() << endl;

			local_map.insert( map<string, int>::value_type(v.name(), num_locals) );
			local_type.push_back( v.type() );

			num_locals += 1;
		}


	}
	out.write_u2(1); // number of attributes - currently only code



	// visit function code
	f->getStmt()->visit(shared_from_this());
	addInstruction(0xb1); // add return


	int codesize = stackSize();
	cout << "code length = " << codesize << endl;

	//	Code_attribute {
	out.write_u2( constant_pool.lookup("Code") ); // "Code"
	out.write_u4( codesize + 12 ); // size of following block

	//	u2 max_stack;
	out.write_u2(10); // just guessing
	cout << "max stack = 10" << endl;

	//	u2 max_locals;
	out.write_u2(num_locals);
	cout << "max locals = " << num_locals << endl;

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
	local_map.clear();
	local_type.clear();

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

void Bytecode::accept(shared_ptr<InitStmt> is) {
	// map name to store number
	local_map.insert( map<string, int>::value_type(is->getVar().name(), num_locals) );
	local_type.push_back( is->getVar().type() );

	// initialising variable
	if (is->hasInit()) {
		t_const = NULL;
		is->getExpr()->visit(shared_from_this());

		if ( !t_const ) {
			throw runtime_error("failure in initial assignment");
		}


		if ( t_const->nameStr() == "int") {
			addInstruction1(0x36, num_locals);			// istore
		}
		else {
			addInstruction1(0x3a, num_locals);			// astore
		}
	}

	num_locals += 1;
}

void Bytecode::accept(shared_ptr<AssignStmt> as) {
	// find name being assigned to
	shared_ptr<Expr> e = as->getLHS();
	shared_ptr<Var> var = e->assignable();
	int ind = local_map[var->name()];
	shared_ptr<Type> t = local_type[ind];

	as->getRHS()->visit( shared_from_this() );	// pushes value

	if ( t->nameStr() == "string") {
		addInstruction1(0x3a, ind);			// astore
	}
	else if ( t->nameStr() == "int") {
		addInstruction1(0x36, ind);			// istore
	}
}


void Bytecode::accept(shared_ptr<IfStmt> is) {
	is->getExpr()->visit( shared_from_this() );
	int instructionNo = istack.size();
	int marker = stackSize();

	addInstruction2(0x99, 0); // ifeq

	is->getBody()->visit( shared_from_this() );
	int diff = stackSize() - marker;

	istack[instructionNo].modifyArg2(diff);
}

void Bytecode::accept(shared_ptr<WhileStmt> ws) {
	int marker1 = stackSize();
	ws->getExpr()->visit( shared_from_this() );

	int marker2 = stackSize();
	int instructionNo = istack.size();
	addInstruction2(0x99, 0); // ifeq

	ws->getBody()->visit( shared_from_this() );

	addInstruction2(0xa7, -(stackSize() - marker1)); // goto

	istack[instructionNo].modifyArg2(stackSize() - marker2);
}

void Bytecode::accept(shared_ptr<ForStmt>) {}

void Bytecode::accept(shared_ptr<PrintStmt> ps) {
	addInstruction2(0xb2, 11); // push output stream

	ps->getExpr()->visit(shared_from_this()); // should add item to print

	if ( t_const->nameStr() == "int") {
		addInstruction2(0xb6, 20); // invoke println for int
	}
	else {
		addInstruction2(0xb6, 17); // invoke println for str
	}
}

void Bytecode::accept(shared_ptr<EvalStmt> es) {
	es->visitChildren( shared_from_this() );
}

void Bytecode::accept(shared_ptr<ReturnStmt>) {}
void Bytecode::accept(shared_ptr<BreakStmt>) {}
void Bytecode::accept(shared_ptr<SwitchStmt>) {}

void Bytecode::accept(shared_ptr<ConstExpr> ex) {
	shared_ptr<Value> v = ex->getValue();
	t_const = v->type();

	int ind = 0;
	if ( t_const->nameStr() == "string") {
		string s = ex->getValue()->asString();
		ind = constant_pool.lookupType("string", s);
	}
	else if ( t_const->nameStr() == "int") {
		shared_ptr<TypedValue<int>> intv = static_pointer_cast<TypedValue<int>, Value>( v );
		int i = intv->value();
		ind = constant_pool.lookup(i);
	}

	if (ind == 0) {
		throw runtime_error("constant pool lookup failed");
	}

	addInstruction1(0x12, ind); // ldc, push constant
}


void Bytecode::accept(shared_ptr<IsTypeExpr>) {}

void Bytecode::accept(shared_ptr<VariableExpr> v) {
	int ind = local_map[v->getVar()->name()];
	shared_ptr<Type> t = local_type[ind];

	if ( t->nameStr() == "int") {
		addInstruction1(0x15, ind); // iload
	}
	else {
		addInstruction1(0x19, ind); // aload
	}
}

void Bytecode::accept(shared_ptr<FuncCallExpr> f) {
	string mname = f->getFunc()->name();
	int ind = constant_pool.lookupType("methodref", mname);
	if (ind == 0) {
		throw runtime_error("constant pool lookup failed");
	}

	// push args
	vector<shared_ptr<Expr>> args = f->getArgs();
	for (shared_ptr<Expr> e: args) {
		e->visit(shared_from_this());
	}

	cout << "invoke " << ind << endl;
	addInstruction2(0xb8, ind); // invokestatic
}

void Bytecode::accept(shared_ptr<RecordExpr>) {}

void Bytecode::accept(shared_ptr<ListExpr> l) {
	addInstruction1(0x10, l->size()); // bipush length

	addInstruction1(0xbc, 10); // newarray int

	for (int i = 0; i < l->size(); ++i) {
		addInstruction(0x59); // dup
		addInstruction1(0x10, i); // bipush index
		l->getExpr(i)->visit( shared_from_this() ); // value
		addInstruction(0x4f); // iastore
	}

	t_const = l->getType();
}


void Bytecode::accept(shared_ptr<ListLengthExpr> l) {
	l->visitChildren( shared_from_this() );
	addInstruction(0xbe); // arraylength

	t_const = l->getType();
}

void Bytecode::accept(shared_ptr<ConcatExpr>) {}

void Bytecode::accept(shared_ptr<ListLookupExpr> ll) {
	ll->getExpr()->visit( shared_from_this() );
	ll->getIndex()->visit( shared_from_this() );
	addInstruction(0x2e); // iaload
}


void Bytecode::accept(shared_ptr<RecordMemberExpr>) {}
void Bytecode::accept(shared_ptr<BasicCastExpr>) {}

void Bytecode::accept(shared_ptr<AbstractOpExpr> oe) {
	oe->getLHS()->visit( shared_from_this() );
	oe->getRHS()->visit( shared_from_this() );


	if ( oe->opcode() == '+') {
		addInstruction(0x60); // iadd
	}

	if ( oe->opcode() == '<') {
		addInstruction2(0xa2, 7); // if_icmpge
		addInstruction(0x4); // iconst_1
		addInstruction2(0xa7, 4); // goto
		addInstruction(0x3); // iconst_0
	}

}

void Bytecode::accept(shared_ptr<EquivOp>) {}
void Bytecode::accept(shared_ptr<NotEquivOp>) {}

void Bytecode::accept(shared_ptr<AndExpr> oe) {
	oe->getLHS()->visit( shared_from_this() );
	oe->getRHS()->visit( shared_from_this() );
	addInstruction(0x7e); // iand
}


void Bytecode::accept(shared_ptr<OrExpr> oe) {
	oe->getLHS()->visit( shared_from_this() );
	oe->getRHS()->visit( shared_from_this() );
	addInstruction(0x80); // ior
}


void Bytecode::accept(shared_ptr<NotExpr>) {}

} /* namespace std */
