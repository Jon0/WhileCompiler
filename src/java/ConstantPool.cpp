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

#include "Bytecode.h"
#include "Classfile.h"
#include "Constant.h"
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

ConstantPool::ConstantPool() {
	add(NULL);	// reserve
//	add(make_shared<UTF8>("java/lang/Object"));		// #1
//	add(make_shared<UTF8>("rubbish"));				// #2
//	add(make_shared<JClass>(1));					// #3
//	add(make_shared<JClass>(2));					// #4
//
//	add(make_shared<UTF8>("Code"));
//
//	// //FieldRef(Class(Utf8("java/lang/System")), NameType(Utf8("out"), Utf8("Ljava/io/PrintStream;")))
//	add(make_shared<UTF8>("java/lang/System")); 	// #6
//	add(make_shared<JClass>(6));					// #7
//	add(make_shared<UTF8>("out"));						// #8
//	add(make_shared<UTF8>("Ljava/io/PrintStream;"));	// #9
//	add(make_shared<JNameType>(8, 9));				// #10
//	add(make_shared<JFieldRef>(7, 10));					// #11
//
//	//MethodRef(Class(Utf8("java/io/PrintStream")), NameType(Utf8("println"), Utf8("(Ljava/lang/String;)V")))
//	add(make_shared<UTF8>("java/io/PrintStream")); 	// #12
//	add(make_shared<JClass>(12));					// #13
//	add(make_shared<UTF8>("println"));					// #14
//	add(make_shared<UTF8>("(Ljava/lang/String;)V"));	// #15
//	add(make_shared<JNameType>(14, 15));				// #16
//	add(make_shared<JMethodRef>(13, 16));				// #17
//
//	// integer version
//	add(make_shared<UTF8>("(I)V"));	// #18
//	add(make_shared<JNameType>(14, 18));				// #19
//	add(make_shared<JMethodRef>(13, 19));				// #20
//
//	// boolean const strings
//	add(make_shared<UTF8>("true"));						// #21
//	add(make_shared<JString>(21));						// #22
//	add(make_shared<UTF8>("false"));					// #23
//	add(make_shared<JString>(23));						// #24
}

ConstantPool::~ConstantPool() {}

short ConstantPool::size() {
	return constant_pool.size();
}

short ConstantPool::use(shared_ptr<Constant> c) {
	if ( !contains(c) ) {
		add(c);
	}
	return find(c);
}

void ConstantPool::add(shared_ptr<Constant> c) {
	if (c) {
		for ( shared_ptr<Constant> cp: c->getChildren() ) {
			add(cp);
		}
	}
	constant_pool.push_back(c);

	if (c) {
		c->setIndex(constant_pool.size() - 1);
		table.insert( map<string, int>::value_type( c->lookupStr(), constant_pool.size() - 1 ) );
	}
}

short ConstantPool::find(shared_ptr<Constant> to_find) {
	for (int i = 0; i < constant_pool.size(); ++i) {
		if ( constant_pool[i] && *constant_pool[i] == *to_find ) return i;
	}
	return -1;
}

bool ConstantPool::contains(shared_ptr<Constant> to_find) {
	for (shared_ptr<Constant> c: constant_pool) {
		if ( c && *c == *to_find ) {
			cout << "match: " << c->toString() << " with " << to_find->toString() << endl;
			return true;
		}
	}
	return false;
}

bytecode ConstantPool::getByteCode() {
	bytecode b;
	//	u2 constant_pool_count;
	write_u2(b, constant_pool.size());

	for (shared_ptr<Constant> c: constant_pool) {
		if (c) {
			write_u1(b, c->getType());

			// for utf8
			write_list(b, c->writeByteCode());

			for (shared_ptr<Constant> cc: c->getChildren()) {
				write_u2(b, find(cc));
			}
		}
	}
	return b;
}

string ConstantPool::debug() {
	string result = "";

	int ind = 0;
	for (shared_ptr<Constant> c: constant_pool) {
		result += "#" + to_string(ind++) + " : ";
		if (c) result += c->toString();
		else result += "null";
		result += "\n";
	}

	return result;
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

// no longer works or used
short ConstantPool::lookup(string s) {
	return table[s];
}

// no longer works or used
short ConstantPool::lookupType(string t, string s) {
	return 0;
}

void ConstantPool::accept(shared_ptr<Func> f) {
	add(make_shared<UTF8>( f->name() ));

	if ( f->name() == "main" ) {
		add(make_shared<UTF8>("([Ljava/lang/String;)V"));
	}
	else {
		string desc = JavaDescriptor( f );
		add(make_shared<UTF8>(desc));
		//add(make_shared<JNameType>( lookup(f->name()), lookup(desc) ));
		//add(make_shared<JMethodRef>(4, lookup(f->name()) + 2)); // TODO fix this.
	}

	//f->getStmt()->visit(shared_from_this());
}

void ConstantPool::accept(shared_ptr<ConstExpr> ex) {
	// const should be a primative type
	shared_ptr<Value> v = ex->getValue();
	shared_ptr<Type> t = v->type();

	if ( t->nameStr() == "string") {
		string s = ex->getValue()->asString();
		add(make_shared<UTF8>(s));
		add(make_shared<JString>( s ));
	}
	else if ( t->nameStr() == "int" || t->nameStr() == "char" ) {
		shared_ptr<TypedValue<int>> intv = static_pointer_cast<TypedValue<int>, Value>( v );
		int i = intv->value();
		add(make_shared<JInteger>(i));
	}
}

} /* namespace std */
