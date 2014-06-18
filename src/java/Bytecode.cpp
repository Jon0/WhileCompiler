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

void Bytecode::accept(shared_ptr<Program> p) {

}

void Bytecode::accept(shared_ptr<Func> f) {

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


		if ( t_const->nameStr() == "int" || t_const->nameStr() == "char" || t_const->nameStr() == "bool") {
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


	if ( t->nameStr() == "int" || t_const->nameStr() == "char" || t->nameStr() == "bool") {
		addInstruction1(0x36, ind);			// istore
	}
	else  {
		addInstruction1(0x3a, ind);			// astore
	}
}


void Bytecode::accept(shared_ptr<IfStmt> is) {
	is->getExpr()->visit( shared_from_this() );
	int instructionNo = istack.size();
	int marker = stackSize();

	addInstruction2(0x99, 0); // ifeq

	is->getBody()->visit( shared_from_this() );
	int diff = stackSize() - marker;


	if (is->hasAlt()) {
		int instructionNo2 = istack.size();
		addInstruction2(0xa7, 0); // goto
		marker = stackSize();

		is->getAlt()->visit( shared_from_this() );

		int diff2 = stackSize() - marker;
		istack[instructionNo].modifyArg2(diff + 3); // include the goto
		istack[instructionNo2].modifyArg2(diff2);
	}
	else {
		istack[instructionNo].modifyArg2(diff);
	}
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

void Bytecode::accept(shared_ptr<ForStmt> fs) {
	fs->getInit()->visit( shared_from_this() );

	int marker1 = stackSize();
	fs->getExpr()->visit( shared_from_this() );

	int marker2 = stackSize();
	int instructionNo = istack.size();
	addInstruction2(0x99, 0); // ifeq

	fs->getBody()->visit( shared_from_this() );
	fs->getInc()->visit( shared_from_this() );
	addInstruction2(0xa7, -(stackSize() - marker1)); // goto

	istack[instructionNo].modifyArg2(stackSize() - marker2);
}

void Bytecode::accept(shared_ptr<PrintStmt> ps) {
	addInstruction2(0xb2, 11); // push output stream

	ps->getExpr()->visit(shared_from_this()); // should add item to print
	shared_ptr<Type> inner_type = ps->getExpr()->getType();

	if ( inner_type->nameStr() == "int" || inner_type->nameStr() == "char") {
		addInstruction2(0xb6, 20); // invoke println for int or char
	}
	else if (inner_type->nameStr() == "bool") {
		addInstruction2(0x99, 8); // ifeq
		addInstruction1(0x12, 22); // ldc, push constant true
		addInstruction2(0xa7, 5); // goto
		addInstruction1(0x12, 24); // ldc, push constant false
		addInstruction2(0xb6, 17); // invoke println for str
	}
	else {
		addInstruction2(0xb6, 17); // invoke println for str
	}
}

void Bytecode::accept(shared_ptr<EvalStmt> es) {
	es->visitChildren( shared_from_this() );
}

void Bytecode::accept(shared_ptr<ReturnStmt> r) {
	r->visitChildren( shared_from_this() );

	if ( r->hasExpr() ) {
		shared_ptr<Type> rtype = r->getExpr()->getType();

		if (rtype->nameStr() == "string") {
			addInstruction(0xb0); // areturn
		}
		else {
			addInstruction(0xac); // ireturn
		}
	}
	else {
		addInstruction(0xb1); // empty return
	}
}


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
	else if ( t_const->nameStr() == "bool") {
		shared_ptr<TypedValue<bool>> intv = static_pointer_cast<TypedValue<bool>, Value>( v );
		bool i = intv->value();
		if (i) {
			addInstruction(0x4); // iconst_1
		}
		else {
			addInstruction(0x3); // iconst_0
		}
		return;
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

	if ( t->nameStr() == "int" || t->nameStr() == "bool" ) {
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
	else if ( oe->opcode() == '-') {
		addInstruction(0x64); // iadd
	}
	else if ( oe->opcode() == '*') {
		addInstruction(0x68); // imul
	}
	else if ( oe->opcode() == '/') {
		addInstruction(0x6c); // idiv
	}
	else if ( oe->opcode() == '%') {
		addInstruction(0x70); // irem
	}
	else if ( oe->opcode() == '<') {
		addInstruction2(0xa2, 7); // if_icmpge
		addInstruction(0x4); // iconst_1
		addInstruction2(0xa7, 4); // goto
		addInstruction(0x3); // iconst_0
	}
	else if ( oe->opcode() == '>') {
		addInstruction2(0xa4, 7); // if_icmple
		addInstruction(0x4); // iconst_1
		addInstruction2(0xa7, 4); // goto
		addInstruction(0x3); // iconst_0
	}
	else if ( oe->opcode() == '[') {
		addInstruction2(0xa3, 7); // if_icmpgt
		addInstruction(0x4); // iconst_1
		addInstruction2(0xa7, 4); // goto
		addInstruction(0x3); // iconst_0
	}
	else if ( oe->opcode() == ']') {
		addInstruction2(0xa1, 7); // if_icmplt
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


void Bytecode::accept(shared_ptr<NotExpr> ne) {
	ne->getExpr()->visit( shared_from_this() );

	addInstruction2(0x99, 7); // ifeq
	addInstruction(0x3); // iconst_0
	addInstruction2(0xa7, 4); // goto
	addInstruction(0x4); // iconst_1
}

} /* namespace std */
