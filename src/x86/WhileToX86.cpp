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

#include "WhileObject.h"
#include "WhileToX86.h"
#include "X86Function.h"
#include "X86Instruction.h"
#include "X86Program.h"

namespace std {

WhileToX86::WhileToX86(shared_ptr<X86Program> o) {
	out = o;
	dLabel = -1;
	tagCount = 1;
	print = make_shared<X86Function>("print", false, true);
	debug_out = true;
}

WhileToX86::~WhileToX86() {}

void WhileToX86::accept(shared_ptr<Program> p) {
	out->initialise( p->getProgramName() );
	p->visitChildren( shared_from_this() );
}

void WhileToX86::accept(shared_ptr<Func> f) {
	refs.clear();
	top.clear();
	bool has_ret = (f->returnType()->nameStr() != "void");
	out->beginFunction( f->name(), has_ret );
	cout << "building " << f->name() << " (" << has_ret << ")" << endl;

	// args should be on top of the stack
	int vi = 16;
	for (Var var: f->getArgs()) {
		string vname = var.name();
		int argsize = getTypeSize(var.type());
		StackSpace ss { vi, argsize };

		shared_ptr<WhileObject> obj = make_shared<WhileObject>(out);
		obj->setLocation( out->getBPRegister(), ss );
		refs.insert( objmap::value_type(vname, obj) );

		vi += argsize;
	}
	if (has_ret) {
		returnSpace = make_shared<WhileObject>(out);
		int argsize = getTypeSize( f->returnType() );
		returnSpace->setLocation( out->getBPRegister(), StackSpace{ vi, argsize } );
	}

	f->visitChildren( shared_from_this() );

	// leave
	out->endFunction();
}

void WhileToX86::accept(shared_ptr<Type>) {}

void WhileToX86::accept(shared_ptr<Value>) {}

void WhileToX86::accept(shared_ptr<BlockStmt> e) {
	e->visitChildren( shared_from_this() );
}

void WhileToX86::accept(shared_ptr<InitStmt> e) {
	if (debug_out) {
		cout << "@init" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@init" ) );
	}

	string vname = e->getVar().name();
	shared_ptr<Type> vtype = e->getVar().type();

	shared_ptr<WhileObject> obj;
	if (e->hasInit()) {
		e->getExpr()->visit(shared_from_this());
		obj = popRef();
	}
	else {
		obj = make_shared<WhileObject>(out);
	}

	obj->putOnStack();
	refs.insert( objmap::value_type(vname, obj) );
}

void WhileToX86::accept(shared_ptr<AssignStmt> e) {
	if (debug_out) {
		cout << "@assign" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@assign" ) );
	}

	e->getLHS()->visit( shared_from_this() );
	shared_ptr<WhileObject> lhs = popRef();

	e->getRHS()->visit( shared_from_this() );
	shared_ptr<WhileObject> rhs = popRef();

	lhs->assign( rhs, true );

	rhs->free();
	lhs->free();
}

void WhileToX86::accept(shared_ptr<IfStmt> e) {
	if (debug_out) {
		cout << "@if" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@if" ) );
	}

	e->getExpr()->visit( shared_from_this() );
	if (top.empty()) {
		cout << "invalid expression" << endl;
		return;
	}

	// top should be a boolean
	//out->addInstruction( "text", make_shared<InstrCmp>( "$0", popRef()->place() ) );
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

void WhileToX86::accept(shared_ptr<WhileStmt>) {
	if (debug_out) {
		cout << "@while" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@while" ) );
	}
}

void WhileToX86::accept(shared_ptr<ForStmt>) {
	if (debug_out) {
		cout << "@forloop" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@forloop" ) );
	}
}

void WhileToX86::accept(shared_ptr<PrintStmt> e) {
	if (debug_out) {
		cout << "@print" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@print" ) );
	}

	e->visitChildren( shared_from_this() );
	shared_ptr<WhileObject> obj = popRef();
	shared_ptr<X86Reference> objAddr = obj->addrRef();
	out->callFunction(print, arg_list{objAddr});
	objAddr->free();
	obj->free();
}

void WhileToX86::accept(shared_ptr<EvalStmt> e) {
	if (debug_out) {
		cout << "@eval" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@eval" ) );
	}

	e->visitChildren( shared_from_this() );
}

void WhileToX86::accept(shared_ptr<ReturnStmt> e) {
	if (debug_out) {
		cout << "@return" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@return" ) );
	}

	e->visitChildren( shared_from_this() );
	if (e->hasExpr()) {
		returnSpace->assign( popRef(), true );
	}
}

void WhileToX86::accept(shared_ptr<BreakStmt>) {
	if (debug_out) {
		cout << "@break" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@break" ) );
	}
}

void WhileToX86::accept(shared_ptr<SwitchStmt>) {
	if (debug_out) {
		cout << "@switch" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@switch" ) );
	}
}

void WhileToX86::accept(shared_ptr<ConstExpr> e) {
	if (debug_out) {
		cout << "@const" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@const" ) );
	}

	shared_ptr<Value> v = e->getValue();
	shared_ptr<WhileObject> obj = make_shared<WhileObject>(out);

	if (v->type()->nameStr() == "string") {
		dLabel++;
		out->addInstruction( "data", make_shared<InstrDirective>( "data" ) );	// TODO shouldnt assume its the first data
		out->addInstruction( "data", make_shared<InstrLabel>( ".LC" + to_string(dLabel) ) );
		out->addInstruction( "data", make_shared<InstrDirective>( "string", vector<string>({v->asString()}) ) );

		//hmmm
		obj->initialise( make_shared<X86Reference>( "$.LC" + to_string(dLabel) ), 7, false );
	}
	else if ( v->type()->nameStr() == "bool") {
		shared_ptr<TypedValue<bool>> b = static_pointer_cast<TypedValue<bool>, Value>( v );
		obj->initialise( make_shared<X86Reference>( b->value() ), 2, false );
	}
	else if ( v->type()->nameStr() == "int") {
		shared_ptr<TypedValue<int>> b = static_pointer_cast<TypedValue<int>, Value>( v );
		obj->initialise( make_shared<X86Reference>( b->value() ), 4, false );
	}
	else if ( v->type()->nameStr() == "real") {
		shared_ptr<TypedValue<double>> b = static_pointer_cast<TypedValue<double>, Value>( v );
		obj->initialise( make_shared<X86Reference>( b->value() ), 5, false );
	}



	top.push_back(obj);
}
void WhileToX86::accept(shared_ptr<IsTypeExpr>) {
	if (debug_out) {
		cout << "@is" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@is" ) );
	}
}

void WhileToX86::accept(shared_ptr<VariableExpr> e) {
	if (debug_out) {
		cout << "@variable" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@variable" ) );
	}

	shared_ptr<Var> a = e->assignable();
	if (a) {
		string vname = a->name();
		if (refs.count(vname) > 0) {
			top.push_back( refs[vname] );
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
	if (debug_out) {
		cout << "@funccall" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@funccall" ) );
	}

	obj_list ol;
	for (shared_ptr<Expr> e: f->getArgs()) {
		e->visit( shared_from_this() );
		ol.push_back( popRef() );
	}

	string fname = f->getFunc()->name();
	shared_ptr<X86Function> x86func = out->getFunction( fname );
	if (!x86func) {
		cout << "function " << fname << " not found" << endl;
		return;
	}
	shared_ptr<WhileObject> ret = out->callFunction( x86func, ol );

	// result gets returned
	if (ret) {
		top.push_back(ret);
	}
}

void WhileToX86::accept(shared_ptr<RecordExpr>) {
	if (debug_out) {
		cout << "@record" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@record" ) );
	}

	shared_ptr<WhileRecord> obj = make_shared<WhileRecord>(out);
	obj->initialise( make_shared<X86Reference>(64), false );

	top.push_back( obj );
}

void WhileToX86::accept(shared_ptr<ListExpr> e) {
	if (debug_out) {
		cout << "@list" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@list" ) );
	}

	int length = e->size();
	shared_ptr<X86Register> reg = out->getFreeRegister();
	reg->assign( make_shared<X86Reference>( length ) );

	shared_ptr<WhileList> obj = make_shared<WhileList>(out);
	obj->initialise( reg->ref(), false );
	reg->free();


	for (int i = 0; i < length; ++i) {
		e->getExpr(i)->visit( shared_from_this() );
		shared_ptr<WhileObject> expr = popRef();
		shared_ptr<WhileObject> obj2 = obj->get<WhileObject>( make_shared<X86Reference>(i) );
		obj2->assign( expr, true );
		obj2->free();
		expr->free();
	}

	top.push_back( obj );
}


void WhileToX86::accept(shared_ptr<ListLengthExpr>) {
	if (debug_out) {
		cout << "@listlength" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@listlength" ) );
	}
}

void WhileToX86::accept(shared_ptr<ConcatExpr>) {
	if (debug_out) {
		cout << "@append" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@append" ) );
	}
}

void WhileToX86::accept(shared_ptr<ListLookupExpr> e) {
	if (debug_out) {
		cout << "@listlookup" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@listlookup" ) );
	}

	// which list
	e->getExpr()->visit( shared_from_this() );
	shared_ptr<WhileList> expr = static_pointer_cast<WhileList, WhileObject>( popRef() );

	// index
	e->getIndex()->visit( shared_from_this() );
	shared_ptr<WhileObject> index = popRef();

	top.push_back( expr->get<WhileObject>( index->valueDirect() ) );
	index->free();
	//expr->free();
}

void WhileToX86::accept(shared_ptr<RecordMemberExpr>) {
	if (debug_out) {
		cout << "@recordmember" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@recordmember" ) );
	}
}

void WhileToX86::accept(shared_ptr<BasicCastExpr> e) {
	if (debug_out) {
		cout << "@cast" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@cast" ) );
	}

	e->visitChildren( shared_from_this() ); // let result be passed back
}

void WhileToX86::accept(shared_ptr<AbstractOpExpr> e) {
	if (debug_out) {
		cout << "@operation" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@operation" ) );
	}

	e->visitChildren( shared_from_this() );



	if ( e->getType()->nameStr() == "bool") {
		//out->addInstruction("text", make_shared<InstrMov>(popRef(), make_shared<X86Reference>(ax, 4)));
		//out->addInstruction("text", make_shared<InstrCmp>(popRef()->place(), "%eax"));
		if (e->opcode() == ']') {
			out->addInstruction("text", make_shared<InstrSet>("ge", "%al"));
		}
		else {
			out->addInstruction("text", make_shared<InstrSet>("g", "%al"));
		}
		out->addInstruction("text", make_shared<InstrMov>("zbl", "%al", "%eax"));
	}
	else {

		shared_ptr<WhileObject> obj = make_shared<WhileObject>(out);
		shared_ptr<WhileObject> rhs = popRef();
		shared_ptr<WhileObject> lhs = popRef();
		obj->assign( lhs, false );	// copy
		out->addInstruction( "text", make_shared<InstrMul>( rhs->valueDirect(), obj->valueDirect() ) ); // TODO register is no longer a reference
	}

	//top.push_back(make_shared<X86Reference>(ax, 4));
}


void WhileToX86::accept(shared_ptr<EquivOp> e) {
	if (debug_out) {
		cout << "@equiv" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@equiv" ) );
	}
	//movl	-20(%rbp), %eax
	//cmpl	-8(%rbp), %eax
	//sete	%al
	//movzbl	%al, %eax


	e->getLHS()->visit( shared_from_this() );
	//out->addInstruction( "text", make_shared<InstrMov>( popRef(), make_shared<X86Reference>(bx, 4) ) );
	e->getRHS()->visit( shared_from_this() );
	//out->addInstruction( "text", make_shared<InstrCmp>( popRef()->place(), "%ebx" ) );
	out->addInstruction( "text", make_shared<InstrSet>( "%al" ) );
	out->addInstruction( "text", make_shared<InstrMov>( "zbl", "%al", "%eax" ) );

	//top.push_back(make_shared<X86Reference>(ax, 4));
}

void WhileToX86::accept(shared_ptr<NotEquivOp>) {
	if (debug_out) {
		cout << "@notequiv" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@notequiv" ) );
	}
}

void WhileToX86::accept(shared_ptr<AndExpr> e) {
	if (debug_out) {
		cout << "@and" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@and" ) );
	}

	e->visitChildren( shared_from_this() );
	//out->addInstruction( "text", make_shared<InstrMov>( popRef(), make_shared<X86Reference>(ax, 4) ) );
	//out->addInstruction( "text", make_shared<InstrMov>( popRef(), make_shared<X86Reference>(dx, 4) ) );
	//out->addInstruction( "text", make_shared<InstrAnd>( "%edx", "%eax" ) );
	//top.push_back(make_shared<X86Reference>(ax, 4));
}

void WhileToX86::accept(shared_ptr<OrExpr>) {
	if (debug_out) {
		cout << "@or" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@or" ) );
	}
}

void WhileToX86::accept(shared_ptr<NotExpr> e) {
	if (debug_out) {
		cout << "@not" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@not" ) );
	}

	e->visitChildren( shared_from_this() );

	//cmpl	$0, -20(%rbp)
	//sete	%al
	//movzbl	%al, %eax
	//movl	%eax, -8(%rbp)

	//out->addInstruction( "text", make_shared<InstrCmp>( "$0", popRef()->place() ) );
	out->addInstruction( "text", make_shared<InstrSet>( "%al" ) );
	out->addInstruction( "text", make_shared<InstrMov>( "zbl", "%al", "%eax" ) );

	//top.push_back(make_shared<X86Reference>(ax, 4));
}

shared_ptr<WhileObject> WhileToX86::popRef() {
	if (top.empty()) {
		cout << "error: stack empty" << endl;
		return make_shared<WhileObject>( out );
	}

	shared_ptr<WhileObject> rr = top.back();
	top.pop_back();
	return rr;
}

shared_ptr<X86Register> WhileToX86::refIntoReg(shared_ptr<X86Reference> ref) {
	shared_ptr<X86Register> reg = out->getFreeRegister();
	reg->assign( ref );
	return reg;
}

int WhileToX86::getTypeSize(shared_ptr<Type> t) {
	// TODO assuming all types are 8+8 bytes
	int ts = 16;
	return ts;
}

void WhileToX86::pushTypeTag(shared_ptr<Type> t) {
	if ( t->isList() ) {
		out->addInstruction( "text", make_shared<InstrPush>( make_shared<X86Reference>( 8 ) ) );
	}
	else {
		out->addInstruction( "text", make_shared<InstrPush>( make_shared<X86Reference>( 4 ) ) );
	}
}

} /* namespace std */
