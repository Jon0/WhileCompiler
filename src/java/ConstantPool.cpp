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

string JavaDescriptor(shared_ptr<Type> t) {
	if (t->nameStr() == "int") {
		return "I";
	}
	else if (t->nameStr() == "void") {
		return "V";
	}
	else if (t->nameStr() == "char") {
		return "C";
	}
	else if (t->nameStr() == "real") {
		return "D";
	}
	else if (t->isList()) {
		shared_ptr<ListType> lt = static_pointer_cast<ListType, Type>( t );

		return "["+JavaDescriptor(lt->innerType());
	}
	else if (t->nameStr() == "string") {
		return "Ljava/lang/String;";
	}
	return "Unknown";
}

string JavaDescriptor(shared_ptr<Func> f) {
	string desc = "(";
	for (int i = 0; i < f->numArgs(); ++i) {
		desc += JavaDescriptor( f->argType(i) );
	}
	desc += ")";
	desc += JavaDescriptor( f->returnType() );
	return desc;
}

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

int JNameType::nameIndex() {
	return name_index;
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

int JMethodRef::nameIndex() {
	return nametype_index;
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

void JInteger::writeByteCode(ClassfileWriter &outfile) {
	outfile.write_u1(3);
	outfile.write_u4(value);
}

string JInteger::lookupStr() {
	return "str";
}

string JInteger::typeStr() {
	return "integer";
}

int JInteger::getValue() {
	return value;
}

void JFloat::writeByteCode(ClassfileWriter &outfile) {
	outfile.write_u1(4);
	outfile.write_u4(value);
}

string JFloat::lookupStr() {
	return "str";
}

string JFloat::typeStr() {
	return "float";
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

short ConstantPool::lookup(int v) {
	for (short i = 0; i < constant_pool.size(); ++i) {
		if (constant_pool[i] && constant_pool[i]->typeStr() == "integer" ) {
			shared_ptr<JInteger> js = static_pointer_cast<JInteger, Constant>( constant_pool[i] );
			if ( js->getValue() == v ) return i;
		}
	}
	return 0;
}

short ConstantPool::lookup(string s) {
	return table[s];
}

short ConstantPool::lookupType(string t, string s) {
	for (short i = 0; i < constant_pool.size(); ++i) {
		if (constant_pool[i] && constant_pool[i]->typeStr() == t && t == "string" ) {
			shared_ptr<JString> js = static_pointer_cast<JString, Constant>( constant_pool[i] );

			if ( constant_pool[js->utf8Index()]->lookupStr() == s ) return i;
		}
		else if (constant_pool[i] && constant_pool[i]->typeStr() == t && t == "methodref" ) {
			shared_ptr<JMethodRef> js = static_pointer_cast<JMethodRef, Constant>( constant_pool[i] );
			shared_ptr<JNameType> jnt = static_pointer_cast<JNameType, Constant>( constant_pool[js->nameIndex()] );

			if ( constant_pool[jnt->nameIndex()]->lookupStr() == s ) return i;

		}
		else if (constant_pool[i] && constant_pool[i]->typeStr() == t && t == "nametype" ) {
			shared_ptr<JNameType> jnt = static_pointer_cast<JNameType, Constant>( constant_pool[i] );

			if ( constant_pool[jnt->nameIndex()]->lookupStr() == s ) return i;
		}
	}
	return 0;
}

short ConstantPool::size() {
	return constant_pool.size();
}

void ConstantPool::accept(shared_ptr<Type>) {}
void ConstantPool::accept(shared_ptr<Value>) {}

void ConstantPool::accept(shared_ptr<Program> p) {
	add(NULL);
	add(make_shared<UTF8>("java/lang/Object"));		// #1
	add(make_shared<UTF8>(p->getProgramName()));	// #2
	add(make_shared<JClass>(1));					// #3
	add(make_shared<JClass>(2));					// #4

	add(make_shared<UTF8>("Code"));

	// //FieldRef(Class(Utf8("java/lang/System")), NameType(Utf8("out"), Utf8("Ljava/io/PrintStream;")))
	add(make_shared<UTF8>("java/lang/System")); 	// #6
	add(make_shared<JClass>(6));					// #7
	add(make_shared<UTF8>("out"));						// #8
	add(make_shared<UTF8>("Ljava/io/PrintStream;"));	// #9
	add(make_shared<JNameType>(8, 9));				// #10
	add(make_shared<JFieldRef>(7, 10));					// #11

	//MethodRef(Class(Utf8("java/io/PrintStream")), NameType(Utf8("println"), Utf8("(Ljava/lang/String;)V")))
	add(make_shared<UTF8>("java/io/PrintStream")); 	// #12
	add(make_shared<JClass>(12));					// #13
	add(make_shared<UTF8>("println"));					// #14
	add(make_shared<UTF8>("(Ljava/lang/String;)V"));	// #15
	add(make_shared<JNameType>(14, 15));				// #16
	add(make_shared<JMethodRef>(13, 16));				// #17

	// integer version
	add(make_shared<UTF8>("(I)V"));	// #18
	add(make_shared<JNameType>(14, 18));				// #19
	add(make_shared<JMethodRef>(13, 19));				// #20

	// boolean const strings
	add(make_shared<UTF8>("true"));						// #21
	add(make_shared<JString>(21));						// #22
	add(make_shared<UTF8>("false"));					// #23
	add(make_shared<JString>(23));						// #24

	FuncMap fm = p->getFuncMap();
	for ( FuncMap::value_type &func: fm ) {
		func.second->visit(shared_from_this());
	}
}

void ConstantPool::accept(shared_ptr<Func> f) {
	add(make_shared<UTF8>( f->name() ));

	if ( f->name() == "main" ) {
		add(make_shared<UTF8>("([Ljava/lang/String;)V"));
	}
	else {
		string desc = JavaDescriptor( f );
		add(make_shared<UTF8>(desc));
		add(make_shared<JNameType>( lookup(f->name()), lookup(desc) ));
		add(make_shared<JMethodRef>(4, lookup(f->name()) + 2)); // TODO fix this.
	}

	f->getStmt()->visit(shared_from_this());
}

void ConstantPool::accept(shared_ptr<BlockStmt> bs) {
	for (shared_ptr<Stmt> s: bs->getStmt()) {
		s->visit(shared_from_this());
	}
}


void ConstantPool::accept(shared_ptr<InitStmt> is) {
	is->visitChildren(shared_from_this());
}

void ConstantPool::accept(shared_ptr<AssignStmt> as) {
	as->getLHS()->visit(shared_from_this());
	as->getRHS()->visit(shared_from_this());
}

void ConstantPool::accept(shared_ptr<IfStmt> is) {
	is->visitChildren( shared_from_this() );
}

void ConstantPool::accept(shared_ptr<WhileStmt> ws) {
	ws->visitChildren( shared_from_this() );
}

void ConstantPool::accept(shared_ptr<ForStmt> fs) {
	fs->visitChildren( shared_from_this() );
}

void ConstantPool::accept(shared_ptr<PrintStmt> ps) {
	ps->getExpr()->visit(shared_from_this());
}

void ConstantPool::accept(shared_ptr<EvalStmt> es) {
	es->visitChildren( shared_from_this() );
}

void ConstantPool::accept(shared_ptr<ReturnStmt> r) {
	r->visitChildren( shared_from_this() );
}

void ConstantPool::accept(shared_ptr<BreakStmt>) {}
void ConstantPool::accept(shared_ptr<SwitchStmt>) {}

void ConstantPool::accept(shared_ptr<ConstExpr> ex) {
	// const should be a primative type
	shared_ptr<Value> v = ex->getValue();
	shared_ptr<Type> t = v->type();

	if ( t->nameStr() == "string") {
		string s = ex->getValue()->asString();
		add(make_shared<UTF8>(s));
		add(make_shared<JString>( lookup(s) ));
	}
	else if ( t->nameStr() == "int" || t->nameStr() == "char" ) {
		shared_ptr<TypedValue<int>> intv = static_pointer_cast<TypedValue<int>, Value>( v );
		int i = intv->value();
		add(make_shared<JInteger>(i));
	}

}

void ConstantPool::accept(shared_ptr<IsTypeExpr>) {}
void ConstantPool::accept(shared_ptr<VariableExpr>) {}

void ConstantPool::accept(shared_ptr<FuncCallExpr> f) {
	// push args
	vector<shared_ptr<Expr>> args = f->getArgs();
	for (shared_ptr<Expr> e: args) {
		e->visit(shared_from_this());
	}
}

void ConstantPool::accept(shared_ptr<RecordExpr>) {}

void ConstantPool::accept(shared_ptr<ListExpr> le) {
	le->visitChildren( shared_from_this() );
}

void ConstantPool::accept(shared_ptr<ListLengthExpr>) {}
void ConstantPool::accept(shared_ptr<ConcatExpr>) {}
void ConstantPool::accept(shared_ptr<ListLookupExpr>) {}
void ConstantPool::accept(shared_ptr<RecordMemberExpr>) {}
void ConstantPool::accept(shared_ptr<BasicCastExpr>) {}

void ConstantPool::accept(shared_ptr<AbstractOpExpr> oe) {
	oe->getLHS()->visit( shared_from_this() );
	oe->getRHS()->visit( shared_from_this() );
}

void ConstantPool::accept(shared_ptr<EquivOp>) {}
void ConstantPool::accept(shared_ptr<NotEquivOp>) {}

void ConstantPool::accept(shared_ptr<AndExpr> oe) {
	oe->getLHS()->visit( shared_from_this() );
	oe->getRHS()->visit( shared_from_this() );
}

void ConstantPool::accept(shared_ptr<OrExpr> oe) {
	oe->getLHS()->visit( shared_from_this() );
	oe->getRHS()->visit( shared_from_this() );
}

void ConstantPool::accept(shared_ptr<NotExpr> ne) {
	ne->getExpr()->visit( shared_from_this() );
}

} /* namespace std */
