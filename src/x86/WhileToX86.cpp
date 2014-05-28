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
	out = o;
	eax = make_shared<X86Register>("eax");
	dLabel = -1;
	tagCount = 1;
	top = NULL;
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

	out->addInstruction( "text", make_shared<InstrGlobl>(f->name()) );	//.globl [function name]
	//out->addInstruction( make_shared<InstrType>(f->name()) );	//.type	[function name], @function

	// labels
	out->addInstruction( "text", make_shared<InstrFuncLabel>( f->name() ) );

	// save stack pointer
	out->addInstruction( "text", make_shared<InstrPush>("%rbp") );
	out->addInstruction( "text", make_shared<InstrMov>("%rsp", "%rbp") );
	out->addInstruction( "text", stack.allocate(16) ); // TODO should modify later

	// args should be on top of the stack, this actually does nothing
	int vi = 0;
	for (Var var: f->getArgs()) {
		string vname = var.name();
		refs.insert( refmap::value_type(vname, make_shared<X86Reference>( stack.nextSpace(4) )) );
		out->addInstruction( "text", make_shared<InstrMov>( to_string(vi)+"(%rbp)", "%eax") );
		out->addInstruction( "text", refs[vname]->setValue( "%eax" ) );
		vi += 4;
	}

	f->visitChildren( shared_from_this() );

	out->addInstruction( "text", make_shared<InstrMov>("%rbp", "%rsp") );
	out->addInstruction( "text", make_shared<InstrPop>("%rbp") );
	out->addInstruction( "text", make_shared<InstrRet>() );
}

void WhileToX86::accept(shared_ptr<Type>) {}

void WhileToX86::accept(shared_ptr<Value>) {}

void WhileToX86::accept(shared_ptr<BlockStmt> e) {
	e->visitChildren( shared_from_this() );
}

void WhileToX86::accept(shared_ptr<InitStmt> e) {
	string vname = e->getVar().name();

	// TODO assuming type is 4 bytes
	refs.insert( refmap::value_type(vname, make_shared<X86Reference>( stack.nextSpace(4) )) );

	// hopefully a reference is made
	top = NULL;
	e->getExpr()->visit( shared_from_this() );
	if (!top) {
		cout << "assignment failed" << endl;
		return;
	}

	out->addInstruction( "text", refs[vname]->setValue( top->place() ) );
}

void WhileToX86::accept(shared_ptr<AssignStmt> e) {
	shared_ptr<Var> a = e->getLHS()->assignable();

	if (a) {
		string vname = a->name();

		out->addInstruction("text", refs[vname]->setValue("$0"));
	}
	else {
		cout << "not assignable" << endl;
	}
}

void WhileToX86::accept(shared_ptr<IfStmt> e) {
	top = NULL;
	e->getExpr()->visit( shared_from_this() );
	if (!top) {
		cout << "invalid expression" << endl;
		return;
	}

	// top should be a boolean
	out->addInstruction( "text", make_shared<InstrCmp>( "$0", top->place() ) );
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
	top = NULL;
	e->visitChildren( shared_from_this() );
	if (!top) return;

	out->addInstruction( "text", make_shared<InstrMov>( top->place(), "%edi" ) );
	//out->addInstruction( "text", make_shared<InstrMov>( "$0", "%eax" ) );

	shared_ptr<Expr> inner = e->getExpr();
	if ( inner->getType()->nameStr() == "bool" ) {
		out->addInstruction( "text", make_shared<InstrCall>("printB") );
	}
	else if ( inner->getType()->nameStr() == "int" ) {
		out->addInstruction( "text", make_shared<InstrCall>( "printI" ) );
	}
	else {
		out->addInstruction( "text", make_shared<InstrCall>( "print" ) );
	}
}

void WhileToX86::accept(shared_ptr<EvalStmt>) {}

void WhileToX86::accept(shared_ptr<ReturnStmt> e) {
	e->visitChildren( shared_from_this() );
	out->addInstruction( "text", make_shared<InstrMov>( top->place(), "%eax" ) );
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
		top = make_shared<X86Reference>( "$.LC" + to_string(dLabel) );

	}
	else if ( v->type()->nameStr() == "bool") {
		shared_ptr<TypedValue<bool>> b = static_pointer_cast<TypedValue<bool>, Value>( v );
		top = make_shared<X86Reference>("$"+to_string(b->value()));
	}
	else if ( v->type()->nameStr() == "int") {
		shared_ptr<TypedValue<int>> b = static_pointer_cast<TypedValue<int>, Value>( v );
		top = make_shared<X86Reference>("$"+to_string(b->value()));
	}

}
void WhileToX86::accept(shared_ptr<IsTypeExpr>) {}

void WhileToX86::accept(shared_ptr<VariableExpr> e) {
	shared_ptr<Var> a = e->assignable();
	if (a) {
		string vname = a->name();
		if (refs.count(vname) > 0) {
			out->addInstruction( "text", refs[vname]->assignRegister(eax) );
			top = refs[vname];
		}
	}
	else {
		// TODO ???
	}
}

void WhileToX86::accept(shared_ptr<FuncCallExpr> f) {

	for (shared_ptr<Expr> e: f->getArgs()) {
		e->visit( shared_from_this() );
		out->addInstruction( "text", make_shared<InstrMov>( top->place(), "%eax" ) );
		out->addInstruction( "text", make_shared<InstrPush>("%rax") );
	}


	out->addInstruction( "text", make_shared<InstrCall>( f->getFunc()->name() ) );
	top = make_shared<X86Reference>(eax);
}

void WhileToX86::accept(shared_ptr<RecordExpr>) {}

void WhileToX86::accept(shared_ptr<ListExpr>) {}
void WhileToX86::accept(shared_ptr<ListLengthExpr>) {}
void WhileToX86::accept(shared_ptr<ConcatExpr>) {}
void WhileToX86::accept(shared_ptr<ListLookupExpr>) {}
void WhileToX86::accept(shared_ptr<RecordMemberExpr>) {}
void WhileToX86::accept(shared_ptr<BasicCastExpr>) {}

void WhileToX86::accept(shared_ptr<AbstractOpExpr>) {
	out->addInstruction( "text", make_shared<InstrMov>( "$345", "%eax" ) );
	out->addInstruction( "text", make_shared<InstrMov>( "$4", "%edx" ) );
	out->addInstruction( "text", make_shared<InstrAdd>( "%edx", "%eax" ) );
	top = make_shared<X86Reference>(eax);
}


void WhileToX86::accept(shared_ptr<EquivOp> e) {
	//movl	-20(%rbp), %eax
	//cmpl	-8(%rbp), %eax
	//sete	%al
	//movzbl	%al, %eax


	e->getLHS()->visit( shared_from_this() );
	out->addInstruction( "text", make_shared<InstrMov>( top->place(), "%ebx" ) );
	e->getRHS()->visit( shared_from_this() );
	out->addInstruction( "text", make_shared<InstrCmp>( top->place(), "%ebx" ) );
	out->addInstruction( "text", make_shared<InstrSet>( "%al" ) );
	out->addInstruction( "text", make_shared<InstrMov>( "zbl", "%al", "%eax" ) );

	top = make_shared<X86Reference>(eax);
}

void WhileToX86::accept(shared_ptr<NotEquivOp>) {}
void WhileToX86::accept(shared_ptr<AndExpr>) {}
void WhileToX86::accept(shared_ptr<OrExpr>) {}

void WhileToX86::accept(shared_ptr<NotExpr>) {
	top = make_shared<X86Reference>(eax);
}

} /* namespace std */
