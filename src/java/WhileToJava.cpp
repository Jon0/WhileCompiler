#include <iostream>

#include "../lang/Program.h"
#include "../lang/Expr.h"
#include "../lang/Func.h"
#include "../lang/Stmt.h"

#include "WhileToJava.h"

namespace std {

WhileToJava::WhileToJava(shared_ptr<Classfile> prog, bool debug)
	:
	classfile(prog), 
	constant_pool(*prog->getConstPool()) {}

WhileToJava::~WhileToJava() {}

int WhileToJava::stackSize() {
	int total = 0;
	for (Instruction &i: istack) {
		total += i.bytes.size();
	}
	return total;
}

void WhileToJava::addInstruction(unsigned char code) {
	Instruction i;
	i.bytes.push_back(code);
	istack.push_back(i);
}

void WhileToJava::addInstruction1(unsigned char code, unsigned char arg) {
	Instruction i;
	i.bytes.push_back(code);
	i.bytes.push_back(arg);
	istack.push_back(i);
}

void WhileToJava::addInstruction2(unsigned char code, unsigned short arg) {
	Instruction i;
	i.bytes.push_back(code);
	i.bytes.push_back((unsigned char) (arg >> 8));
	i.bytes.push_back((unsigned char) (arg >> 0));
	istack.push_back(i);
}

void WhileToJava::addInstruction4(unsigned char code, unsigned int arg) {
	Instruction i;
	i.bytes.push_back(code);
	i.bytes.push_back((unsigned char) (arg >> 24));
	i.bytes.push_back((unsigned char) (arg >> 16));
	i.bytes.push_back((unsigned char) (arg >> 8));
	i.bytes.push_back((unsigned char) (arg >> 0));
	istack.push_back(i);
}

void WhileToJava::accept(shared_ptr<lang::Type>) {}
void WhileToJava::accept(shared_ptr<lang::Value>) {}

void WhileToJava::accept(shared_ptr<lang::Program> p) {
	classfile->initialise( p->getProgramName() );

	// go to functions
	p->visitChildren( shared_from_this() );
}

void WhileToJava::accept(shared_ptr<lang::Func> f) {
	classfile->beginFunction("main");
	classfile->call();
}

void WhileToJava::accept(shared_ptr<lang::BlockStmt> bs) {
	for (shared_ptr<lang::Stmt> s: bs->getStmt()) {
		s->visit(shared_from_this());
	}
}

void WhileToJava::accept(shared_ptr<lang::InitStmt> is) {
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

void WhileToJava::accept(shared_ptr<lang::AssignStmt> as) {
	// find name being assigned to
	auto e = as->getLHS();
	auto var = e->assignable();
	int ind = local_map[var->name()];
	auto t = local_type[ind];

	as->getRHS()->visit( shared_from_this() );	// pushes value


	if ( t->nameStr() == "int" || t_const->nameStr() == "char" || t->nameStr() == "bool") {
		addInstruction1(0x36, ind);			// istore
	}
	else  {
		addInstruction1(0x3a, ind);			// astore
	}
}


void WhileToJava::accept(shared_ptr<lang::IfStmt> is) {
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

void WhileToJava::accept(shared_ptr<lang::WhileStmt> ws) {
	int marker1 = stackSize();
	ws->getExpr()->visit( shared_from_this() );

	int marker2 = stackSize();
	int instructionNo = istack.size();
	addInstruction2(0x99, 0); // ifeq

	ws->getBody()->visit( shared_from_this() );

	addInstruction2(0xa7, -(stackSize() - marker1)); // goto

	istack[instructionNo].modifyArg2(stackSize() - marker2);
}

void WhileToJava::accept(shared_ptr<lang::ForStmt> fs) {
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

void WhileToJava::accept(shared_ptr<lang::PrintStmt> ps) {
	addInstruction2(0xb2, 11); // push output stream

	ps->getExpr()->visit(shared_from_this()); // should add item to print
	auto inner_type = ps->getExpr()->getType();

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

void WhileToJava::accept(shared_ptr<lang::EvalStmt> es) {
	es->visitChildren( shared_from_this() );
}

void WhileToJava::accept(shared_ptr<lang::ReturnStmt> r) {
	r->visitChildren( shared_from_this() );

	if ( r->hasExpr() ) {
		auto rtype = r->getExpr()->getType();

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


void WhileToJava::accept(shared_ptr<lang::BreakStmt>) {}
void WhileToJava::accept(shared_ptr<lang::SwitchStmt>) {}

void WhileToJava::accept(shared_ptr<lang::ConstExpr> ex) {
	auto v = ex->getValue();
	t_const = v->type();
	int ind = 0;
	if ( t_const->nameStr() == "string") {
		string s = ex->getValue()->asString();
		ind = constant_pool.lookupType("string", s);
	}
	else if ( t_const->nameStr() == "int") {
		auto intv = static_pointer_cast<lang::TypedValue<int>, lang::Value>( v );
		int i = intv->value();
		ind = constant_pool.lookup(i);
	}
	else if ( t_const->nameStr() == "bool") {
		auto intv = static_pointer_cast<lang::TypedValue<bool>, lang::Value>( v );
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


void WhileToJava::accept(shared_ptr<lang::IsTypeExpr>) {}

void WhileToJava::accept(shared_ptr<lang::VariableExpr> v) {
	int ind = local_map[v->getVar()->name()];
	auto t = local_type[ind];

	if ( t->nameStr() == "int" || t->nameStr() == "bool" ) {
		addInstruction1(0x15, ind); // iload
	}
	else {
		addInstruction1(0x19, ind); // aload
	}
}

void WhileToJava::accept(shared_ptr<lang::FuncCallExpr> f) {
	string mname = f->getFunc()->name();
	int ind = constant_pool.lookupType("methodref", mname);
	if (ind == 0) {
		throw runtime_error("constant pool lookup failed");
	}

	// push args
	auto args = f->getArgs();
	for (shared_ptr<lang::Expr> e: args) {
		e->visit(shared_from_this());
	}

	addInstruction2(0xb8, ind); // invokestatic
}

void WhileToJava::accept(shared_ptr<lang::RecordExpr>) {}

void WhileToJava::accept(shared_ptr<lang::ListExpr> l) {
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


void WhileToJava::accept(shared_ptr<lang::ListLengthExpr> l) {
	l->visitChildren( shared_from_this() );
	addInstruction(0xbe); // arraylength

	t_const = l->getType();
}

void WhileToJava::accept(shared_ptr<lang::ConcatExpr>) {}

void WhileToJava::accept(shared_ptr<lang::ListLookupExpr> ll) {
	ll->getExpr()->visit( shared_from_this() );
	ll->getIndex()->visit( shared_from_this() );
	addInstruction(0x2e); // iaload
}


void WhileToJava::accept(shared_ptr<lang::RecordMemberExpr>) {}
void WhileToJava::accept(shared_ptr<lang::BasicCastExpr>) {}

void WhileToJava::accept(shared_ptr<lang::AbstractOpExpr> oe) {
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

void WhileToJava::accept(shared_ptr<lang::EquivOp>) {}
void WhileToJava::accept(shared_ptr<lang::NotEquivOp>) {}

void WhileToJava::accept(shared_ptr<lang::AndExpr> oe) {
	oe->getLHS()->visit( shared_from_this() );
	oe->getRHS()->visit( shared_from_this() );
	addInstruction(0x7e); // iand
}


void WhileToJava::accept(shared_ptr<lang::OrExpr> oe) {
	oe->getLHS()->visit( shared_from_this() );
	oe->getRHS()->visit( shared_from_this() );
	addInstruction(0x80); // ior
}


void WhileToJava::accept(shared_ptr<lang::NotExpr> ne) {
	ne->getExpr()->visit( shared_from_this() );

	addInstruction2(0x99, 7); // ifeq
	addInstruction(0x3); // iconst_0
	addInstruction2(0xa7, 4); // goto
	addInstruction(0x4); // iconst_1
}

} /* namespace std */
