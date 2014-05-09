/*
 * ConstantPool.cpp
 *
 *  Created on: 9/05/2014
 *      Author: remnanjona
 */

#include "../lang/Program.h"
#include "../lang/Expr.h"
#include "../lang/Func.h"
#include "../lang/Stmt.h"

#include "ConstantPool.h"

namespace std {

void UTF8::writeByteCode(ClassfileWriter &outfile) {
	outfile.write_u1(1);
	outfile.write_u2(str.length());
	outfile.write_str(str);
}

string UTF8::lookupStr() {
	return str;
}

string UTF8::typeStr() {
	return "utf8";
}

void JClass::writeByteCode(ClassfileWriter &outfile) {
	outfile.write_u1(7);
	outfile.write_u2(ind);
}

string JClass::lookupStr() {
	return "class";
}

string JClass::typeStr() {
	return "class";
}

void JNameType::writeByteCode(ClassfileWriter &outfile) {
	outfile.write_u1(12);
	outfile.write_u2(name_index);
	outfile.write_u2(descriptor_index);
}

string JNameType::lookupStr() {
	return "nd";
}

string JNameType::typeStr() {
	return "nametype";
}


void JFieldRef::writeByteCode(ClassfileWriter &outfile) {
	outfile.write_u1(9);
	outfile.write_u2(class_index);
	outfile.write_u2(nametype_index);
}

string JFieldRef::lookupStr() {
	return "fr";
}

string JFieldRef::typeStr() {
	return "fieldref";
}

void JMethodRef::writeByteCode(ClassfileWriter &outfile) {
	outfile.write_u1(10);
	outfile.write_u2(class_index);
	outfile.write_u2(nametype_index);
}

string JMethodRef::lookupStr() {
	return "mr";
}

string JMethodRef::typeStr() {
	return "methodref";
}

void JString::writeByteCode(ClassfileWriter &outfile) {
	outfile.write_u1(8);
	outfile.write_u2(utf8_index);
}

string JString::lookupStr() {
	return "str";
}

string JString::typeStr() {
	return "string";
}

int JString::utf8Index() {
	return utf8_index;
}

ConstantPool::ConstantPool() {}

ConstantPool::~ConstantPool() {
	// TODO Auto-generated destructor stub
}

void ConstantPool::writeByteCode(ClassfileWriter &out) {
	//	u2 constant_pool_count;
	out.write_u2(constant_pool.size());

	for (shared_ptr<Constant> c: constant_pool) {
		if (c) c->writeByteCode(out);
	}
}

void ConstantPool::add(shared_ptr<Constant> c) {
	constant_pool.push_back(c);

	if (c) {
		table.insert( map<string, int>::value_type( c->lookupStr(), constant_pool.size() - 1 ) );
	}
}

short ConstantPool::lookup(string s) {
	return table[s];
}

short ConstantPool::lookupType(string, string s) {
	for (short i = 0; i < constant_pool.size(); ++i) {
		if (constant_pool[i] && constant_pool[i]->typeStr() == "string") {
			shared_ptr<JString> js = static_pointer_cast<JString, Constant>( constant_pool[i] );

			if ( constant_pool[js->utf8Index()]->lookupStr() == s ) return i;
		}
	}
}

short ConstantPool::size() {
	return constant_pool.size();
}

void ConstantPool::accept(shared_ptr<Type>) {}
void ConstantPool::accept(shared_ptr<Value>) {}

void ConstantPool::accept(shared_ptr<Program> p) {
	add(NULL);
	add(make_shared<UTF8>("java/lang/Object"));
	add(make_shared<UTF8>(p->getProgramName()));
	add(make_shared<JClass>(1));
	add(make_shared<JClass>(2));

	add(make_shared<UTF8>("main"));
	add(make_shared<UTF8>("([Ljava/lang/String;)V"));

	add(make_shared<UTF8>("Code"));

	// //FieldRef(Class(Utf8("java/lang/System")), NameType(Utf8("out"), Utf8("Ljava/io/PrintStream;")))
	add(make_shared<UTF8>("java/lang/System")); 	// #8
	add(make_shared<JClass>(8));					// #9
	add(make_shared<UTF8>("out"));						// #10
	add(make_shared<UTF8>("Ljava/io/PrintStream;"));	// #11
	add(make_shared<JNameType>(10, 11));				// #12
	add(make_shared<JFieldRef>(9, 12));					// #13

	//MethodRef(Class(Utf8("java/io/PrintStream")), NameType(Utf8("println"), Utf8("(Ljava/lang/String;)V")))
	add(make_shared<UTF8>("java/io/PrintStream")); 	// #14
	add(make_shared<JClass>(14));					// #15
	add(make_shared<UTF8>("println"));					// #16
	add(make_shared<UTF8>("(Ljava/lang/String;)V"));	// #17
	add(make_shared<JNameType>(16, 17));				// #18
	add(make_shared<JMethodRef>(15, 18));				// #19

	FuncMap fm = p->getFuncMap();
	for ( FuncMap::value_type &func: fm ) {
		func.second->visit(shared_from_this());
	}
}

void ConstantPool::accept(shared_ptr<Func> f) {
	f->getStmt()->visit(shared_from_this());
}

void ConstantPool::accept(shared_ptr<BlockStmt> bs) {
	for (shared_ptr<Stmt> s: bs->getStmt()) {
		s->visit(shared_from_this());
	}
}


void ConstantPool::accept(shared_ptr<InitStmt>) {}
void ConstantPool::accept(shared_ptr<AssignStmt>) {}
void ConstantPool::accept(shared_ptr<IfStmt>) {}
void ConstantPool::accept(shared_ptr<WhileStmt>) {}
void ConstantPool::accept(shared_ptr<ForStmt>) {}

void ConstantPool::accept(shared_ptr<PrintStmt> ps) {
	ps->getExpr()->visit(shared_from_this());
}

void ConstantPool::accept(shared_ptr<EvalStmt>) {}
void ConstantPool::accept(shared_ptr<ReturnStmt>) {}
void ConstantPool::accept(shared_ptr<BreakStmt>) {}
void ConstantPool::accept(shared_ptr<SwitchStmt>) {}

void ConstantPool::accept(shared_ptr<ConstExpr> ex) {

	string s = ex->getValue()->asString();

	add(make_shared<UTF8>(s));
	add(make_shared<JString>( lookup(s) ));

}

void ConstantPool::accept(shared_ptr<IsTypeExpr>) {}
void ConstantPool::accept(shared_ptr<VariableExpr>) {}
void ConstantPool::accept(shared_ptr<FuncCallExpr>) {}
void ConstantPool::accept(shared_ptr<RecordExpr>) {}

void ConstantPool::accept(shared_ptr<ListExpr>) {}
void ConstantPool::accept(shared_ptr<ListLengthExpr>) {}
void ConstantPool::accept(shared_ptr<ConcatExpr>) {}
void ConstantPool::accept(shared_ptr<ListLookupExpr>) {}
void ConstantPool::accept(shared_ptr<RecordMemberExpr>) {}
void ConstantPool::accept(shared_ptr<BasicCastExpr>) {}
void ConstantPool::accept(shared_ptr<AbstractOpExpr>) {}

} /* namespace std */
