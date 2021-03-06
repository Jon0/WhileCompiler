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
	clone_objs = false;
	debug_out = add_debug;
}

WhileToX86::~WhileToX86() {}

void WhileToX86::accept(shared_ptr<lang::Program> p) {
	out->initialise( p->getProgramName() );

	function_list flist;
	lang::FuncMap fmap = p->getFuncMap();
	for ( auto &e: fmap ) {
		string fname = e.first;
		bool has_return = (e.second->returnType()->nameStr() != "void");
		flist.push_back( make_shared<X86Function>(fname, has_return, false) );
	}
	out->declareFunctions(flist);

	p->visitChildren( shared_from_this() );

	if (debug_out) cout << "completed build" << endl;
}

void WhileToX86::accept(shared_ptr<lang::Func> f) {
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
		returnSpace = make_obj( out, f->returnType() );
		unsigned int argsize = WhileObject::getTypeSize( f->returnType() );
		returnSpace->setLocation( mem_space{ out->getDIRegister()->ref(vi), argsize }, false );
		vi -= argsize;
	}
	int argnum = 0;
	for (lang::Var &var: f->getArgs()) {
		string vname = var.name();
		unsigned int argsize = WhileObject::getTypeSize(var.type());

		shared_ptr<WhileObject> obj = make_obj(out, f->argType(argnum++));
		obj->setLocation( mem_space{ out->getDIRegister()->ref(vi), argsize }, false );
		refs.insert( objmap::value_type(vname, obj) );
		vi -= argsize;
	}

	f->visitChildren( shared_from_this() );

	// add return for void functions
	if (!has_ret) out->endFunction();
}

void WhileToX86::accept(shared_ptr<lang::Type>) {}

void WhileToX86::accept(shared_ptr<lang::Value>) {}

void WhileToX86::accept(shared_ptr<lang::BlockStmt> e) {
	e->visitChildren( shared_from_this() );
}

void WhileToX86::accept(shared_ptr<lang::InitStmt> e) {
	if (debug_out) {
		cout << "@init" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@init" ) );
	}

	string vname = e->getVar().name();
	auto vtype = e->getVar().type();

	shared_ptr<WhileObject> obj;
	if (e->hasInit()) {
		obj = objFromExpr( e->getExpr(), true );
	}
	else {
		obj = make_obj(out, vtype);
	}
	obj->putOnStack(); // TODO is this needed if obj is cloned?

	refs.insert( {vname, obj} );
}

void WhileToX86::accept(shared_ptr<lang::AssignStmt> e) {
	if (debug_out) {
		cout << "@assign" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@assign" ) );
	}

	shared_ptr<WhileObject> lhs = objFromExpr( e->getLHS(), false );
	shared_ptr<WhileObject> rhs = objFromExpr( e->getRHS(), true );
	lhs->assign( rhs, true );
}

void WhileToX86::accept(shared_ptr<lang::IfStmt> e) {
	if (debug_out) {
		cout << "@if" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@if" ) );
	}
	e->getExpr()->visit( shared_from_this() );

	// top should be a boolean
	shared_ptr<X86Register> r = popRef()->attachRegister();
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

void WhileToX86::accept(shared_ptr<lang::WhileStmt> e) {
	if (debug_out) {
		cout << "@while" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@while" ) );
	}
	string tag1 = ".L"+to_string(tagCount++);
	loopbreak = ".L"+to_string(tagCount++);
	out->addInstruction( "text", make_shared<InstrLabel>( tag1 ) );

	e->getExpr()->visit( shared_from_this() );

	// top should be a boolean
	shared_ptr<X86Register> r = popRef()->attachRegister();
	r->compare( make_shared<X86ConstRef>( 0 ) );

	out->addInstruction( "text", make_shared<InstrJ>( "e", loopbreak ) );
	e->getBody()->visit( shared_from_this() );
	out->addInstruction( "text", make_shared<InstrJ>( "mp", tag1 ) );

	out->addInstruction( "text", make_shared<InstrLabel>( loopbreak ) );
}

void WhileToX86::accept(shared_ptr<lang::ForStmt> e) {
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
	shared_ptr<X86Register> r = popRef()->attachRegister();
	r->compare( make_shared<X86ConstRef>( 0 ) );


	out->addInstruction( "text", make_shared<InstrJ>( "e", loopbreak ) );
	e->getBody()->visit( shared_from_this() );
	e->getInc()->visit( shared_from_this() );
	out->addInstruction( "text", make_shared<InstrJ>( "mp", tag1 ) );

	out->addInstruction( "text", make_shared<InstrLabel>( loopbreak ) );
}

void WhileToX86::accept(shared_ptr<lang::PrintStmt> e) {
	if (debug_out) {
		cout << "@print" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@print" ) );
	}
	objFromExpr( e->getExpr() )->print();
}

void WhileToX86::accept(shared_ptr<lang::EvalStmt> e) {
	if (debug_out) {
		cout << "@eval" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@eval" ) );
	}

	e->visitChildren( shared_from_this() );
}

void WhileToX86::accept(shared_ptr<lang::ReturnStmt> e) {
	if (debug_out) {
		cout << "@return" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@return" ) );
	}

	e->visitChildren( shared_from_this() );
	if (e->hasExpr()) {
		returnSpace->assign( popRef(), true );
	}

	out->endFunction();
}

void WhileToX86::accept(shared_ptr<lang::BreakStmt>) {
	if (debug_out) {
		cout << "@break" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@break" ) );
	}

	// the loop will already make the tag
	out->addInstruction( "text", make_shared<InstrJ>( "mp", loopbreak ) );
}

void WhileToX86::accept(shared_ptr<lang::SwitchStmt> e) {
	if (debug_out) {
		cout << "@switch" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@switch" ) );
	}

	e->getSwitch()->visit( shared_from_this() );
	shared_ptr<WhileObject> obj1 = popRef();

	vector<string> labels;
	string defLabel;
	loopbreak = ".L"+to_string(tagCount++);

	// build jump table
	for (auto c: e->getCases()) {
		labels.push_back( ".L"+to_string(tagCount++) );
		shared_ptr<WhileObject> e_obj = obj1->equiv( objFromExpr(c.first) );
		shared_ptr<X86Register> r = e_obj->attachRegister();
		r->compare( make_shared<X86ConstRef>( 0 ) );
		out->addInstruction( "text", make_shared<InstrJ>( "ne", labels.back() ) );
	}
	if (e->hasDefCase()) {
		defLabel = ".L"+to_string(tagCount++);
		out->addInstruction( "text", make_shared<InstrJ>( "mp", defLabel ) );
	}
	else {
		out->addInstruction( "text", make_shared<InstrJ>( "mp", loopbreak ) );
	}

	// label targets
	int lcount = 0;
	for (auto c: e->getCases()) {
		out->addInstruction( "text", make_shared<InstrLabel>( labels[lcount++] ) );
		c.second->visit( shared_from_this() );
	}
	if (e->hasDefCase()) {
		out->addInstruction( "text", make_shared<InstrLabel>( defLabel ) );
		e->getDefCase()->visit( shared_from_this() );
	}

	out->addInstruction( "text", make_shared<InstrLabel>( loopbreak ) );
}

void WhileToX86::accept(shared_ptr<lang::ConstExpr> e) {
	if (debug_out) {
		cout << "@const" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@const" ) );
	}

	auto v = e->getValue();
	shared_ptr<WhileObject> obj;

	if ( v->type()->nameStr() == "null") {
		obj = make_shared<WhileObject>(out, v->type());
		obj->initialise( make_shared<X86ConstRef>( 0 ), false );
	}
	else if ( v->type()->nameStr() == "bool") {
		obj = make_shared<WhileObject>(out, v->type());
		auto b = static_pointer_cast<lang::TypedValue<bool>, lang::Value>( v );
		obj->initialise( make_shared<X86ConstRef>( b->value() ), false );
	}
	else if ( v->type()->nameStr() == "char") {
		obj = make_shared<WhileObject>(out, v->type());
		auto b = static_pointer_cast<lang::TypedValue<char>, lang::Value>( v );
		obj->initialise( make_shared<X86ConstRef>( b->value() ), false );
	}
	else if ( v->type()->nameStr() == "int") {
		obj = make_shared<WhileObject>(out, v->type());
		auto b = static_pointer_cast<lang::TypedValue<int>, lang::Value>( v );
		obj->initialise( make_shared<X86ConstRef>( b->value() ), false );
	}
	else if ( v->type()->nameStr() == "real") {
		obj = make_shared<WhileObject>(out, v->type());
		auto b = static_pointer_cast<lang::TypedValue<double>, lang::Value>( v );

		// mmx register needs loading from memory?
		//obj->putOnStack();
		obj->initialise( make_shared<X86RealRef>( b->value() ), false ); // true
	}
	else if (v->type()->nameStr() == "string") {
		auto objl = make_shared<WhileList>(out, v->type());
		string text = v->asString();

		// TODO shouldnt assume its the first data
//		dLabel++;
//		out->addInstruction( "data", make_shared<InstrDirective>( "data" ) );
//		out->addInstruction( "data", make_shared<InstrLabel>( ".LC" + to_string(dLabel) ) );
//		out->addInstruction( "data", make_shared<InstrDirective>( "string", vector<string>({text}) ) );
//		obj->initialise( make_shared<X86Reference>( "$.LC" + to_string(dLabel) ), 7, false );

		int length = text.length();
		shared_ptr<X86Register> reg = out->getFreeRegister();
		reg->assign( make_shared<X86ConstRef>( length ) );

		objl->initialise( reg->ref(), false );
		objl->assignType( v->type(), false );

		// set characters
		for (int i = 0; i < length; ++i) {
			shared_ptr<WhileObject> obj2 = objl->get<WhileObject>( make_shared<X86ConstRef>(i) );
			obj2->initialise( make_shared<X86ConstRef>(text[i]), true );
		}

		obj = objl;
	}

	top.push_back(obj);
}
void WhileToX86::accept(shared_ptr<lang::IsTypeExpr> e) {
	if (debug_out) {
		cout << "@is" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@is" ) );
	}

	e->visitChildren( shared_from_this() );

	shared_ptr<WhileObject> obj = popRefAndCopy();
	shared_ptr<X86Register> r = obj->attachRegisterType();
	r->compare( make_shared<X86ConstRef>( WhileObject::getTypeTag( e->getRHS() ) ) );

	shared_ptr<X86Register> r2 = obj->attachRegister();
	r2->setFromFlags("e");

	obj->assignType( lang::boolType, false ); // bool result

	top.push_back( obj );
}

void WhileToX86::accept(shared_ptr<lang::VariableExpr> e) {
	if (debug_out) {
		cout << "@variable" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@variable" ) );
	}

	auto a = e->assignable();
	string vname = a->name();
	auto vtype = a->type();
	if (refs.count(vname) > 0) {
		if (clone_objs) {
			// clone list types
			top.push_back(refs[vname]->clone());
		}
		else {
			top.push_back( refs[vname] );
		}
	} else {
		throw runtime_error("variable " + vname + " not named");
	}

}

void WhileToX86::accept(shared_ptr<lang::FuncCallExpr> f) {
	if (debug_out) {
		cout << "@funccall" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@funccall" ) );
	}

	obj_list ol;
	for (auto &e: f->getArgs()) {
		//e->visit( shared_from_this() );
		//ol.push_back( popRef() );

		ol.push_back( objFromExpr( e, true ) ); // clone items
	}

	string fname = f->getFunc()->name();
	shared_ptr<X86Function> x86func = out->getFunction( fname );
	if (!x86func) {
		cout << "function " << fname << " not found" << endl;
		return;
	}
	shared_ptr<WhileObject> ret = out->callFunction( x86func, f->getFunc()->returnType(), ol );

	// result gets returned
	if (ret) {
		top.push_back(ret);
	}
}

void WhileToX86::accept(shared_ptr<lang::RecordExpr> e) {
	if (debug_out) {
		cout << "@record" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@record" ) );
	}

	shared_ptr<WhileRecord> obj = make_shared<WhileRecord>(out, e->getType());
	obj->initialise( make_shared<X86ConstRef>(64), false );
	top.push_back( obj );
}

void WhileToX86::accept(shared_ptr<lang::ListExpr> e) {
	if (debug_out) {
		cout << "@list" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@list" ) );
	}

	int length = e->size();
	shared_ptr<X86Register> reg = out->getFreeRegister();
	reg->assign( make_shared<X86ConstRef>( length ) );

	shared_ptr<WhileList> obj = make_shared<WhileList>(out, e->getType());
	obj->initialise( reg->ref(), false );

	for (int i = 0; i < length; ++i) {
		e->getExpr(i)->visit( shared_from_this() );
		shared_ptr<WhileObject> expr = popRef();
		shared_ptr<WhileObject> obj2 = obj->get( make_shared<X86ConstRef>(i) );
		obj2->assign( expr, true );
	}

	top.push_back( obj );
}


void WhileToX86::accept(shared_ptr<lang::ListLengthExpr> e) {
	if (debug_out) {
		cout << "@listlength" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@listlength" ) );
	}

	// which list
	e->getExpr()->visit( shared_from_this() );
	shared_ptr<WhileList> expr = static_pointer_cast<WhileList, WhileObject>( popRef() );
	shared_ptr<WhileObject> l = make_obj(out, e->getType());
	expr->length(l);
	top.push_back( l );
}

void WhileToX86::accept(shared_ptr<lang::ConcatExpr> e) {
	if (debug_out) {
		cout << "@append" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@append" ) );
	}

	e->visitChildren( shared_from_this() );
	shared_ptr<WhileObject> r = popRef();
	shared_ptr<WhileObject> l = popRef();
	top.push_back( l->append( r ) );
}

void WhileToX86::accept(shared_ptr<lang::ListLookupExpr> e) {
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
	top.push_back( expr->get( index->valueDirect() ) );
}

void WhileToX86::accept(shared_ptr<lang::RecordMemberExpr>) {
	if (debug_out) {
		cout << "@recordmember" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@recordmember" ) );
	}
}

// cannot change between list and non list types
void WhileToX86::accept(shared_ptr<lang::BasicCastExpr> e) {
	if (debug_out) {
		cout << "@cast" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@cast" ) );
	}

	shared_ptr<WhileObject> inner = objFromExpr( e->getExpr() );

	// modify type
	inner->assignType(e->getType(), false);

	top.push_back(inner);
}

void WhileToX86::accept(shared_ptr<lang::AbstractOpExpr> e) {
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

		obj->assignType( lang::boolType, false ); // bool result
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


void WhileToX86::accept(shared_ptr<lang::EquivOp> e) {
	if (debug_out) {
		cout << "@equiv" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@equiv" ) );
	}

	e->visitChildren( shared_from_this() );
	top.push_back( popRef()->equiv(popRef()) );
}

void WhileToX86::accept(shared_ptr<lang::NotEquivOp> e) {
	if (debug_out) {
		cout << "@notequiv" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@notequiv" ) );
	}
	e->visitChildren( shared_from_this() );
	shared_ptr<WhileObject> obj = popRef()->equiv(popRef());

	// negate r
	shared_ptr<X86Register> r = obj->attachRegister();
	r->compare( make_shared<X86ConstRef>( 0 ) );
	r->setFromFlags("e"); // equal to zero
	top.push_back( obj );
}

void WhileToX86::accept( shared_ptr<lang::AndExpr> e ) {
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

void WhileToX86::accept( shared_ptr<lang::OrExpr> e ) {
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

void WhileToX86::accept(shared_ptr<lang::NotExpr> e) {
	if (debug_out) {
		cout << "@not" << endl;
		out->addInstruction( "text", make_shared<InstrComment>( "@not" ) );
	}

	e->visitChildren( shared_from_this() );
	shared_ptr<WhileObject> obj = make_shared<WhileObject>(out, e->getType());
	shared_ptr<WhileObject> inner = popRef();
	obj->assign( inner, false );	// copy
	shared_ptr<X86Register> r = obj->attachRegister();
	r->compare( make_shared<X86ConstRef>( 0 ) );
	r->setFromFlags("e"); // equal to zero

	top.push_back( obj );
}

shared_ptr<WhileObject> WhileToX86::popRef() {
	if (top.empty()) {
		throw runtime_error("error: stack empty");
	}

	shared_ptr<WhileObject> rr = top.back();
	top.pop_back();
	return rr;
}

shared_ptr<WhileObject> WhileToX86::popRefAndCopy() {
	shared_ptr<WhileObject> lhs = popRef();
	shared_ptr<WhileObject> obj = make_obj(out, lhs->getType());
	obj->assign( lhs, false );	// copy
	return obj;
}

shared_ptr<WhileObject>  WhileToX86::objFromExpr( shared_ptr<lang::Expr> e, bool clone ) {
	clone_objs = clone;
	e->visit( shared_from_this() );
	clone_objs = false;
	return popRef();
}

shared_ptr<WhileObject>  WhileToX86::objFromExpr( shared_ptr<lang::Expr> e ) {
	e->visit( shared_from_this() );
	return popRef();
}

shared_ptr<X86Register> WhileToX86::refIntoReg(shared_ptr<X86Reference> ref) {
	shared_ptr<X86Register> reg = out->getFreeRegister();
	reg->assign( ref );
	return reg;
}

} /* namespace std */
