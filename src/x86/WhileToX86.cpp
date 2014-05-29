/*
 * WhileToX86.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "../lang/Program.h"
#include "../lang/Expr.h"
#include "../lang/Func.h"
#include "../lang/Stmt.h"

#include "WhileToX86.h"
#include "X86Instruction.h"
#include "X86Program.h"

namespace std {

WhileToX86::WhileToX86(shared_ptr<X86Program> o) {
	addr_size = 8;
	out = o;
	ax = make_shared<X86Register>("ax");
	bx = make_shared<X86Register>("bx");
	cx = make_shared<X86Register>("cx");
	dx = make_shared<X86Register>("dx");
	di = make_shared<X86Register>("di");
	sp = make_shared<X86Register>("sp");
	bp = make_shared<X86Register>("bp");
	dLabel = -1;
	tagCount = 1;
}

WhileToX86::~WhileToX86() {
	// TODO Auto-generated destructor stub
}

void WhileToX86::accept(shared_ptr<Program> p) {
	out->setName( p->getProgramName() );
	out->addInstruction( "file", make_shared<InstrDirective>( "file", vector<string>({p->getProgramName()}) ) );
	out->addInstruction( "text", make_shared<InstrDirective>( "text" ) );

	p->visitChildren( shared_from_this() );
}

void WhileToX86::accept(shared_ptr<Func> f) {
	refs.clear();
	top.clear();
	stack.clear();
	cout << "building " << f->name() << endl;

	out->addInstruction( "text", make_shared<InstrGlobl>(f->name()) );	//.globl [function name]
	//out->addInstruction( make_shared<InstrType>(f->name()) );	//.type	[function name], @function

	// label function
	out->addInstruction( "text", make_shared<InstrFuncLabel>( f->name() ) );

	// save stack pointer

	out->addInstruction( "text", make_shared<InstrPush>(make_shared<X86Reference>(bp, addr_size)) );
	out->addInstruction( "text", make_shared<InstrMov>(make_shared<X86Reference>(sp, addr_size), make_shared<X86Reference>(bp, addr_size)) );
	out->addInstruction( "text", stack.allocate() ); // TODO should modify later

	// args should be on top of the stack
	int vi = 16;
	for (Var var: f->getArgs()) {
		string vname = var.name();
		int argsize = getTypeSize(var.type());
		StackSpace ss { vi, argsize, var.type() };
		refs.insert( refmap::value_type(vname, make_shared<X86Reference>( ss )) );
		vi += argsize;
	}

	f->visitChildren( shared_from_this() );

	// leave
	out->addInstruction( "text", make_shared<InstrMov>(make_shared<X86Reference>(bp, addr_size), make_shared<X86Reference>(sp, addr_size)) );
	out->addInstruction( "text", make_shared<InstrPop>("%rbp") );
	out->addInstruction( "text", make_shared<InstrRet>() );
}

void WhileToX86::accept(shared_ptr<Type>) {}

void WhileToX86::accept(shared_ptr<Value>) {}

void WhileToX86::accept(shared_ptr<BlockStmt> e) {
	e->visitChildren( shared_from_this() );
}

void WhileToX86::accept(shared_ptr<InitStmt> e) {
	cout << "@init" << endl;
	string vname = e->getVar().name();
	shared_ptr<Type> vtype = e->getVar().type();

	int ts = getTypeSize(vtype);
	refs.insert( refmap::value_type(vname, make_shared<X86Reference>( stack.nextSpace(vtype, ts) )) );

	// set type info
	out->addInstruction( "text", make_shared<InstrMov>( make_shared<X86Reference>("$"+to_string(2)), refs[vname] ) );

	if (e->hasInit()) {
		// hopefully a reference is made
		e->getExpr()->visit(shared_from_this());
		if (top.empty()) {
			cout << "assignment failed" << endl;
			return;
		}
		out->addInstruction("text", refs[vname]->setValue( popRef() ));
	}
}

void WhileToX86::accept(shared_ptr<AssignStmt> e) {
	shared_ptr<Var> a = e->getLHS()->assignable();

	if (a) {
		string vname = a->name();

		// hopefully a reference is made
		e->getRHS()->visit( shared_from_this() );
		if (top.empty()) {
			cout << "assignment failed" << endl;
			return;
		}

		out->addInstruction("text", refs[vname]->setValue( popRef() ));
	}
	else {
		cout << "not assignable" << endl;
	}
}

void WhileToX86::accept(shared_ptr<IfStmt> e) {
	cout << "@if" << endl;

	e->getExpr()->visit( shared_from_this() );
	if (top.empty()) {
		cout << "invalid expression" << endl;
		return;
	}

	// top should be a boolean
	out->addInstruction( "text", make_shared<InstrCmp>( "$0", popRef()->place() ) );
	string tag1 = ".L"+to_string(tagCount++);
	string tag2 = ".L"+to_string(tagCount++);

	out->addInstruction( "text", make_shared<InstrJ>( "e", tag1 ) );
	e->getBody()->visit( shared_from_this() );
	out->addInstruction( "text", make_shared<InstrJ>( "mp", tag2 ) );

	out->addInstruction( "text", make_shared<InstrLabel>( tag1 ) );
	if (e->hasAlt()) {
		e->getAlt()->visit( shared_from_this() );
	}
	out->addInstruction( "text", make_shared<InstrLabel>( tag2 ) );
}

void WhileToX86::accept(shared_ptr<WhileStmt>) {}

void WhileToX86::accept(shared_ptr<ForStmt>) {}

void WhileToX86::accept(shared_ptr<PrintStmt> e) {
	cout << "@print" << endl;

	e->visitChildren( shared_from_this() );
	if (top.empty()) return;

	// must be put into di register
	shared_ptr<X86Reference> r = popRef();
	out->addInstruction( "text", make_shared<InstrMov>( r, make_shared<X86Reference>(di, r->typeSize()) ) );

	shared_ptr<Expr> inner = e->getExpr();
	if ( inner->getType()->nameStr() == "bool" ) {
		out->addInstruction( "text", make_shared<InstrCall>("printB") );
	}
	else if ( inner->getType()->nameStr() == "int" ) {
		out->addInstruction( "text", make_shared<InstrCall>( "printI" ) );
	}
	else if ( inner->getType()->nameStr() == "string" ) {
		out->addInstruction( "text", make_shared<InstrCall>( "print" ) );
	}
	else if ( inner->getType()->isList() ) {
		out->addInstruction( "text", make_shared<InstrCall>("printL") );
	}
}

void WhileToX86::accept(shared_ptr<EvalStmt> e) {
	e->visitChildren( shared_from_this() );
}

void WhileToX86::accept(shared_ptr<ReturnStmt> e) {
	e->visitChildren( shared_from_this() );
	if (e->hasExpr()) {
		shared_ptr<X86Reference> r = popRef();
		out->addInstruction( "text", r->assignRegister(ax) );
	}
}

void WhileToX86::accept(shared_ptr<BreakStmt>) {}
void WhileToX86::accept(shared_ptr<SwitchStmt>) {}

void WhileToX86::accept(shared_ptr<ConstExpr> e) {
	shared_ptr<Value> v = e->getValue();

	if (v->type()->nameStr() == "string") {
		dLabel++;
		out->addInstruction( "data", make_shared<InstrDirective>( "data" ) );	// TODO shouldnt assume its the first data
		out->addInstruction( "data", make_shared<InstrLabel>( ".LC" + to_string(dLabel) ) );
		out->addInstruction( "data", make_shared<InstrDirective>( "string", vector<string>({v->asString()}) ) );
		top.push_back(make_shared<X86Reference>( "$.LC" + to_string(dLabel) ));

	}
	else if ( v->type()->nameStr() == "bool") {
		shared_ptr<TypedValue<bool>> b = static_pointer_cast<TypedValue<bool>, Value>( v );
		top.push_back(make_shared<X86Reference>("$"+to_string(b->value())));
	}
	else if ( v->type()->nameStr() == "int") {
		shared_ptr<TypedValue<int>> b = static_pointer_cast<TypedValue<int>, Value>( v );
		top.push_back(make_shared<X86Reference>("$"+to_string(b->value())));
	}

}
void WhileToX86::accept(shared_ptr<IsTypeExpr>) {}

void WhileToX86::accept(shared_ptr<VariableExpr> e) {
	shared_ptr<Var> a = e->assignable();
	if (a) {
		string vname = a->name();
		if (refs.count(vname) > 0) {
			top.push_back(refs[vname]);
		}
		else {
			cout << "variable not named" << endl;
		}
	}
	else {
		cout << "not a variable" << endl;
		// TODO ???
	}
}

void WhileToX86::accept(shared_ptr<FuncCallExpr> f) {
	cout << "@funccall" << endl;

	for (shared_ptr<Expr> e: f->getArgs()) {
		e->visit( shared_from_this() );
		shared_ptr<X86Reference> ref = popRef();
		shared_ptr<X86Register> reg = refIntoReg(ref);
		out->addInstruction( "text", make_shared<InstrPush>( make_shared<X86Reference>( reg, ref->typeSize() ) ) );
	}

	out->addInstruction( "text", make_shared<InstrCall>( f->getFunc()->name() ) );

	// result gets returned on eax
	top.push_back(make_shared<X86Reference>(ax, 4));
}

void WhileToX86::accept(shared_ptr<RecordExpr>) {}

void WhileToX86::accept(shared_ptr<ListExpr> e) {
	int msize = 4 + e->size() * 4;
	out->addInstruction( "text", make_shared<InstrMov>( make_shared<X86Reference>("$"+to_string(msize)), make_shared<X86Reference>(di, 4) ) );
	out->addInstruction( "text", make_shared<InstrCall>( "malloc" ) );

	e->visitChildren( shared_from_this() );

	// should have right number on stack
	for (int i = 0; i < e->size(); ++i) {
		out->addInstruction("text", make_shared<InstrMov>(popRef(), make_shared<X86Reference>(to_string(msize-(i+1)*4)+"(%rax)")));
	}
	out->addInstruction("text", make_shared<InstrMov>(make_shared<X86Reference>("$"+to_string(e->size())), make_shared<X86Reference>("0(%rax)")));

	// address returned in rax
	top.push_back(make_shared<X86Reference>(ax, 8));
}


void WhileToX86::accept(shared_ptr<ListLengthExpr>) {}
void WhileToX86::accept(shared_ptr<ConcatExpr>) {}

void WhileToX86::accept(shared_ptr<ListLookupExpr>) {
	top.push_back(make_shared<X86Reference>("$1"));
}

void WhileToX86::accept(shared_ptr<RecordMemberExpr>) {}
void WhileToX86::accept(shared_ptr<BasicCastExpr>) {}

void WhileToX86::accept(shared_ptr<AbstractOpExpr> e) {
	e->visitChildren( shared_from_this() );

	if ( e->getType()->nameStr() == "bool") {
		out->addInstruction("text", make_shared<InstrMov>(popRef(), make_shared<X86Reference>(ax, 4)));
		out->addInstruction("text", make_shared<InstrCmp>(popRef()->place(), "%eax"));
		if (e->opcode() == ']') {
			out->addInstruction("text", make_shared<InstrSet>("ge", "%al"));
		}
		else {
			out->addInstruction("text", make_shared<InstrSet>("g", "%al"));
		}
		out->addInstruction("text", make_shared<InstrMov>("zbl", "%al", "%eax"));
	}
	else {
		out->addInstruction( "text", make_shared<InstrMov>( popRef(), make_shared<X86Reference>(ax, 4) ) );
		out->addInstruction( "text", make_shared<InstrAdd>( popRef()->place(), "%eax" ) ); // TODO register is no longer a reference
	}
	top.push_back(make_shared<X86Reference>(ax, 4));
}


void WhileToX86::accept(shared_ptr<EquivOp> e) {
	//movl	-20(%rbp), %eax
	//cmpl	-8(%rbp), %eax
	//sete	%al
	//movzbl	%al, %eax


	e->getLHS()->visit( shared_from_this() );
	out->addInstruction( "text", make_shared<InstrMov>( popRef(), make_shared<X86Reference>(bx, 4) ) );
	e->getRHS()->visit( shared_from_this() );
	out->addInstruction( "text", make_shared<InstrCmp>( popRef()->place(), "%ebx" ) );
	out->addInstruction( "text", make_shared<InstrSet>( "%al" ) );
	out->addInstruction( "text", make_shared<InstrMov>( "zbl", "%al", "%eax" ) );

	top.push_back(make_shared<X86Reference>(ax, 4));
}

void WhileToX86::accept(shared_ptr<NotEquivOp>) {}

void WhileToX86::accept(shared_ptr<AndExpr> e) {
	e->visitChildren( shared_from_this() );
	out->addInstruction( "text", make_shared<InstrMov>( popRef(), make_shared<X86Reference>(ax, 4) ) );
	out->addInstruction( "text", make_shared<InstrMov>( popRef(), make_shared<X86Reference>(dx, 4) ) );
	out->addInstruction( "text", make_shared<InstrAnd>( "%edx", "%eax" ) );
	top.push_back(make_shared<X86Reference>(ax, 4));
}

void WhileToX86::accept(shared_ptr<OrExpr>) {}

void WhileToX86::accept(shared_ptr<NotExpr> e) {
	e->visitChildren( shared_from_this() );

	//cmpl	$0, -20(%rbp)
	//sete	%al
	//movzbl	%al, %eax
	//movl	%eax, -8(%rbp)

	out->addInstruction( "text", make_shared<InstrCmp>( "$0", popRef()->place() ) );
	out->addInstruction( "text", make_shared<InstrSet>( "%al" ) );
	out->addInstruction( "text", make_shared<InstrMov>( "zbl", "%al", "%eax" ) );

	top.push_back(make_shared<X86Reference>(ax, 4));
}

shared_ptr<X86Reference> WhileToX86::popRef() {
	if (top.empty()) {
		cout << "error: stack empty" << endl;
		return make_shared<X86Reference>( "$0" );
	}

	shared_ptr<X86Reference> rr = top.back();
	top.pop_back();
	return rr;
}

shared_ptr<X86Register> WhileToX86::getFreeRegister() {
	return ax;
}

shared_ptr<X86Register> WhileToX86::refIntoReg(shared_ptr<X86Reference> ref) {
	shared_ptr<X86Register> reg = getFreeRegister();
	out->addInstruction( "text", make_shared<InstrMov>( ref, make_shared<X86Reference>( reg, ref->typeSize() ) ) );
	return reg;
}

int WhileToX86::getTypeSize(shared_ptr<Type> t) {

	// TODO assuming type is 4+4 bytes
	int ts = 8;
	if (t->isList()) {
		ts = 12;
	}

	return ts;
}


} /* namespace std */
