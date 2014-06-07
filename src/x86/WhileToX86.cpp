/*
 * WhileToX86.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include <cstring>
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

WhileToX86::WhileToX86(shared_ptr<X86Program> o, bool add_debug) {
	out = o;
	dLabel = -1;
	tagCount = 1;
	print = make_shared<X86Function>("print", false, true);
	equiv = make_shared<X86Function>("equiv", true, true);
	append = make_shared<X86Function>("append", true, true);
	debug_out = add_debug;
}

WhileToX86::~WhileToX86() {}

void WhileToX86::accept(shared_ptr<Program> p) {
	out->initialise( p->getProgramName() );

	function_list flist;
	FuncMap fmap = p->getFuncMap();
	for ( FuncMap::value_type e: fmap ) {
		string fname = e.first;
		bool has_return = (e.second->returnType()->nameStr() != "void");
		flist.push_back( make_shared<X86Function>(fname, has_return, false) );
	}
	out->declareFunctions(flist);

	p->visitChildren( shared_from_this() );
}

void WhileToX86::accept(shared_ptr<Func> f) {
	refs.clear();
	top.clear();
	bool has_ret = (f->returnType()->nameStr() != "void");
	out->beginFunction( f->name(), has_ret );

	if (debug_out) {
		cout << "building " << f->name() << " (" << has_ret << ")" << endl;
	}

	// args should be on top of the stack
	int vi = 0;
	if (has_ret) {
		returnSpace = make_shared<WhileObject>(out);
		unsigned int argsize = getTypeSize( f->returnType() );
		returnSpace->setLocation( mem_space{ out->getDIRegister()->ref(vi), argsize } );
		vi -= argsize;
	}
	for (Var var: f->getArgs()) {
		string vname = var.name();
		unsigned int argsize = getTypeSize(var.type());

		shared_ptr<WhileObject> obj = make_shared<WhileObject>(out);
		obj->setLocation( mem_space{ out->getDIRegister()->ref(vi), argsize } );
		refs.insert( objmap::value_type(vname, obj) );
		vi -= argsize;
	}

	f->visitChildren( shared_from_this() );

	// add return for void functions
	if (!has_ret) out->endFunction();
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
}

void WhileToX86::accept(shared_ptr<IfStmt> e) {
	if (debug_out) {
		cout << "@if" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@if" ) );
	}
	e->getExpr()->visit( shared_from_this() );

	// top should be a boolean
	shared_ptr<WhileObject> obj = popRef();
	shared_ptr<X86Register> r = obj->attachRegister();
	r->compare( make_shared<X86ConstRef>( 0 ) );

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

void WhileToX86::accept(shared_ptr<WhileStmt> e) {
	if (debug_out) {
		cout << "@while" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@while" ) );
	}
	string tag1 = ".L"+to_string(tagCount++);
	loopbreak = ".L"+to_string(tagCount++);
	out->addInstruction( "text", make_shared<InstrLabel>( tag1 ) );

	e->getExpr()->visit( shared_from_this() );

	// top should be a boolean
	shared_ptr<WhileObject> obj = popRef();
	shared_ptr<X86Register> r = obj->attachRegister();
	r->compare( make_shared<X86ConstRef>( 0 ) );

	out->addInstruction( "text", make_shared<InstrJ>( "e", loopbreak ) );
	e->getBody()->visit( shared_from_this() );
	out->addInstruction( "text", make_shared<InstrJ>( "mp", tag1 ) );

	out->addInstruction( "text", make_shared<InstrLabel>( loopbreak ) );
}

void WhileToX86::accept(shared_ptr<ForStmt> e) {
	if (debug_out) {
		cout << "@forloop" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@forloop" ) );
	}
	e->getInit()->visit( shared_from_this() );

	string tag1 = ".L"+to_string(tagCount++);
	loopbreak = ".L"+to_string(tagCount++);
	out->addInstruction( "text", make_shared<InstrLabel>( tag1 ) );

	e->getExpr()->visit( shared_from_this() );

	// top should be a boolean
	shared_ptr<WhileObject> obj = popRef();
	shared_ptr<X86Register> r = obj->attachRegister();
	r->compare( make_shared<X86ConstRef>( 0 ) );

	out->addInstruction( "text", make_shared<InstrJ>( "e", loopbreak ) );
	e->getBody()->visit( shared_from_this() );
	e->getInc()->visit( shared_from_this() );
	out->addInstruction( "text", make_shared<InstrJ>( "mp", tag1 ) );

	out->addInstruction( "text", make_shared<InstrLabel>( loopbreak ) );
}

void WhileToX86::accept(shared_ptr<PrintStmt> e) {
	if (debug_out) {
		cout << "@print" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@print" ) );
	}

	e->visitChildren( shared_from_this() );
	shared_ptr<WhileObject> obj = popRef();
	shared_ptr<X86RegRef> objAddr = obj->addrRef();
	out->callFunction(print, arg_list{objAddr});
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
		shared_ptr<WhileObject> result = popRef();
		returnSpace->assign( result, true );
	}

	out->endFunction();
}

void WhileToX86::accept(shared_ptr<BreakStmt>) {
	if (debug_out) {
		cout << "@break" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@break" ) );
	}

	// the loop will already make the tag
	out->addInstruction( "text", make_shared<InstrJ>( "mp", loopbreak ) );
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
	shared_ptr<WhileObject> obj;

	if ( v->type()->nameStr() == "null") {
		obj = make_shared<WhileObject>(out);
		obj->initialise( make_shared<X86ConstRef>( 0 ), 1, false );
	}
	else if ( v->type()->nameStr() == "bool") {
		obj = make_shared<WhileObject>(out);
		shared_ptr<TypedValue<bool>> b = static_pointer_cast<TypedValue<bool>, Value>( v );
		obj->initialise( make_shared<X86ConstRef>( b->value() ), 2, false );
	}
	else if ( v->type()->nameStr() == "char") {
		obj = make_shared<WhileObject>(out);
		shared_ptr<TypedValue<char>> b = static_pointer_cast<TypedValue<char>, Value>( v );
		obj->initialise( make_shared<X86ConstRef>( b->value() ), 3, false );
	}
	else if ( v->type()->nameStr() == "int") {
		obj = make_shared<WhileObject>(out);
		shared_ptr<TypedValue<int>> b = static_pointer_cast<TypedValue<int>, Value>( v );
		obj->initialise( make_shared<X86ConstRef>( b->value() ), 4, false );
	}
	else if ( v->type()->nameStr() == "real") {
		obj = make_shared<WhileObject>(out);
		shared_ptr<TypedValue<double>> b = static_pointer_cast<TypedValue<double>, Value>( v );
		long r;
		memcpy(&r, &b->value(), 4);
		obj->initialise( make_shared<X86ConstRef>( r ), 5, false );
	}
	else if (v->type()->nameStr() == "string") {
		shared_ptr<WhileList> objl = make_shared<WhileList>(out);
		string text = v->asString();

//		dLabel++;
//		out->addInstruction( "data", make_shared<InstrDirective>( "data" ) );	// TODO shouldnt assume its the first data
//		out->addInstruction( "data", make_shared<InstrLabel>( ".LC" + to_string(dLabel) ) );
//		out->addInstruction( "data", make_shared<InstrDirective>( "string", vector<string>({text}) ) );
//		obj->initialise( make_shared<X86Reference>( "$.LC" + to_string(dLabel) ), 7, false );

		int length = text.length();
		shared_ptr<X86Register> reg = out->getFreeRegister();
		reg->assign( make_shared<X86ConstRef>( length ) );

		objl->initialise( reg->ref(), false );
		objl->modifyType(7);

		// set characters
		for (int i = 0; i < length; ++i) {
			shared_ptr<WhileObject> obj2 = objl->get<WhileObject>( make_shared<X86ConstRef>(i) );

			obj2->initialise( make_shared<X86ConstRef>(text[i]), 3, true );
		}

		obj = objl;
	}

	top.push_back(obj);
}
void WhileToX86::accept(shared_ptr<IsTypeExpr> e) {
	if (debug_out) {
		cout << "@is" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@is" ) );
	}

	e->visitChildren( shared_from_this() );

	shared_ptr<WhileObject> obj = popRefAndCopy();
	shared_ptr<X86Register> r = obj->attachRegisterType();
	r->compare( make_shared<X86ConstRef>( getTypeTag( e->getRHS() ) ) );

	shared_ptr<X86Register> r2 = obj->attachRegister();
	r2->setFromFlags("e");

	obj->modifyType(2); // bool result

	top.push_back( obj );
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
		cout << "not a variable" << endl; // ???
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
	obj->initialise( make_shared<X86ConstRef>(64), false );
	top.push_back( obj );
}

void WhileToX86::accept(shared_ptr<ListExpr> e) {
	if (debug_out) {
		cout << "@list" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@list" ) );
	}

	int length = e->size();
	shared_ptr<X86Register> reg = out->getFreeRegister();
	reg->assign( make_shared<X86ConstRef>( length ) );

	shared_ptr<WhileList> obj = make_shared<WhileList>(out);
	obj->initialise( reg->ref(), false );

	for (int i = 0; i < length; ++i) {
		e->getExpr(i)->visit( shared_from_this() );
		shared_ptr<WhileObject> expr = popRef();
		shared_ptr<WhileObject> obj2 = obj->get<WhileObject>( make_shared<X86ConstRef>(i) );
		obj2->assign( expr, true );
	}

	top.push_back( obj );
}


void WhileToX86::accept(shared_ptr<ListLengthExpr> e) {
	if (debug_out) {
		cout << "@listlength" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@listlength" ) );
	}

	// which list
	e->getExpr()->visit( shared_from_this() );
	shared_ptr<WhileList> expr = static_pointer_cast<WhileList, WhileObject>( popRef() );

	shared_ptr<WhileObject> l = make_shared<WhileObject>(out);
	expr->length(l);
	top.push_back( l );

	//expr->free();
}

void WhileToX86::accept(shared_ptr<ConcatExpr> e) {
	if (debug_out) {
		cout << "@append" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@append" ) );
	}

	e->visitChildren( shared_from_this() );
	shared_ptr<WhileObject> obj1 = popRef();
	shared_ptr<X86RegRef> objAddr1 = obj1->addrRef();

	shared_ptr<WhileObject> obj2 = popRef();
	shared_ptr<X86RegRef> objAddr2 = obj2->addrRef();

	// use a c function to compare objects
	shared_ptr<X86Register> r = out->callFunction(append, arg_list{objAddr1, objAddr2});

	shared_ptr<WhileObject> obj = make_shared<WhileObject>(out);
	obj->setLocation( r );	// list result
	top.push_back( obj );
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

	// TODO modify type

}

void WhileToX86::accept(shared_ptr<AbstractOpExpr> e) {
	if (debug_out) {
		cout << "@operation" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@operation" ) );
	}
	e->visitChildren( shared_from_this() );
	shared_ptr<WhileObject> rhs = popRef();
	shared_ptr<WhileObject> obj = popRefAndCopy();	// copy lhs
	shared_ptr<X86Register> r = obj->attachRegister();

	if ( e->getType()->nameStr() == "bool") {
		r->compare( rhs->valueDirect() );

		if (e->opcode() == ']') {
			r->setFromFlags("ge"); // greater or equal
		}
		else if (e->opcode() == '>') {
			r->setFromFlags("g"); // greater
		}
		else if (e->opcode() == '[') {
			r->setFromFlags("le"); // less or equal
		}
		else if (e->opcode() == '<') {
			r->setFromFlags("l"); // less
		}

		obj->modifyType(2); // bool result
	}
	else {
		if (e->opcode() == '+') {
			r->add( rhs->valueDirect() );
		}
		else if (e->opcode() == '-') {
			r->sub( rhs->valueDirect() );
		}
		else if (e->opcode() == '*') {
			r->multiply( rhs->valueDirect() );
		}
		else if (e->opcode() == '/') {
			r->divide( rhs->valueDirect() );
		}
		else if (e->opcode() == '%') {
			r->mod( rhs->valueDirect() );
		}
	}
	top.push_back( obj );
}


void WhileToX86::accept(shared_ptr<EquivOp> e) {
	if (debug_out) {
		cout << "@equiv" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@equiv" ) );
	}

	e->visitChildren( shared_from_this() );
	shared_ptr<WhileObject> obj1 = popRef();
	shared_ptr<X86RegRef> objAddr1 = obj1->addrRef();

	shared_ptr<WhileObject> obj2 = popRef();
	shared_ptr<X86RegRef> objAddr2 = obj2->addrRef();

	// use a c function to compare objects
	shared_ptr<X86Register> r = out->callFunction(equiv, arg_list{objAddr1, objAddr2});

	shared_ptr<WhileObject> obj = make_shared<WhileObject>(out);
	obj->initialise( r->ref(), 2, false );	// bool result
	top.push_back( obj );
}

void WhileToX86::accept(shared_ptr<NotEquivOp> e) {
	if (debug_out) {
		cout << "@notequiv" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@notequiv" ) );
	}

	e->visitChildren( shared_from_this() );
	shared_ptr<WhileObject> obj1 = popRef();
	shared_ptr<X86RegRef> objAddr1 = obj1->addrRef();

	shared_ptr<WhileObject> obj2 = popRef();
	shared_ptr<X86RegRef> objAddr2 = obj2->addrRef();

	shared_ptr<X86Register> r = out->callFunction(equiv, arg_list{objAddr1, objAddr2});

	// negate r
	r->compare( make_shared<X86ConstRef>( 0 ) );
	r->setFromFlags("ne"); // not equal

	shared_ptr<WhileObject> obj = make_shared<WhileObject>(out);
	obj->initialise( r->ref(), 2, false );	// bool result
	top.push_back( obj );
}

void WhileToX86::accept( shared_ptr<AndExpr> e ) {
	if (debug_out) {
		cout << "@and" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@and" ) );
	}

	e->visitChildren( shared_from_this() );

	shared_ptr<WhileObject> rhs = popRef();
	shared_ptr<WhileObject> obj = popRefAndCopy();
	shared_ptr<X86Register> r = obj->attachRegister();
	r->andBitwise( rhs->valueDirect() );

	top.push_back( obj );
}

void WhileToX86::accept( shared_ptr<OrExpr> e ) {
	if (debug_out) {
		cout << "@or" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@or" ) );
	}

	e->visitChildren( shared_from_this() );
	shared_ptr<WhileObject> rhs = popRef();
	shared_ptr<WhileObject> obj = popRefAndCopy();
	shared_ptr<X86Register> r = obj->attachRegister();
	r->orBitwise( rhs->valueDirect() );
	top.push_back( obj );
}

void WhileToX86::accept(shared_ptr<NotExpr> e) {
	if (debug_out) {
		cout << "@not" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@not" ) );
	}

	e->visitChildren( shared_from_this() );
	shared_ptr<WhileObject> obj = make_shared<WhileObject>(out);
	shared_ptr<WhileObject> inner = popRef();
	obj->assign( inner, false );	// copy
	shared_ptr<X86Register> r = obj->attachRegister();
	r->compare( make_shared<X86ConstRef>( 0 ) );
	r->setFromFlags("e"); // equal to zero

	top.push_back( obj );
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

shared_ptr<WhileObject> WhileToX86::popRefAndCopy() {
	shared_ptr<WhileObject> obj = make_shared<WhileObject>(out);
	shared_ptr<WhileObject> lhs = popRef();
	obj->assign( lhs, false );	// copy
	return obj;
}

shared_ptr<X86Register> WhileToX86::refIntoReg(shared_ptr<X86Reference> ref) {
	shared_ptr<X86Register> reg = out->getFreeRegister();
	reg->assign( ref );
	return reg;
}

int WhileToX86::getTypeSize(shared_ptr<Type> t) {
	// all while types are 8+8 bytes
	return 16;
}

int WhileToX86::getTypeTag(shared_ptr<Type> t) {
	if ( t->nameStr() == "null") {
		return 1;
	}
	else if ( t->nameStr() == "bool") {
		return 2;
	}
	else if ( t->nameStr() == "char") {
		return 3;
	}
	else if ( t->nameStr() == "int") {
		return 4;
	}
	else if ( t->nameStr() == "real") {
		return 5;
	}
	else if (t->nameStr() == "string") {
		return 7;
	}
	else if (t->isList()) {
		return 8;
	}
	else if (t->isRecord()) {
		return 16;
	}
	return 0;
}

} /* namespace std */
