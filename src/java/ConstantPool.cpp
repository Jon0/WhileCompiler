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

string JavaDescriptor(shared_ptr<lang::Type> t) {
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
		auto lt = static_pointer_cast<lang::ListType, lang::Type>( t );

		return "["+JavaDescriptor(lt->innerType());
	}
	else if (t->nameStr() == "string") {
		return "Ljava/lang/String;";
	}
	return "Unknown";
}

string JavaDescriptor(shared_ptr<lang::Func> f) {
	string desc = "(";
	for (int i = 0; i < f->numArgs(); ++i) {
		desc += JavaDescriptor( f->argType(i) );
	}
	desc += ")";
	desc += JavaDescriptor( f->returnType() );
	return desc;
}

ConstantPool::ConstantPool() {
	add(NULL);	// reserve as in java spec
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
		table.insert( {c->lookupStr(), constant_pool.size() - 1} );
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
			auto js = static_pointer_cast<JInteger, Constant>( constant_pool[i] );
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

void ConstantPool::accept(shared_ptr<lang::Func> f) {
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

void ConstantPool::accept(shared_ptr<lang::ConstExpr> ex) {
	// const should be a primative type
	auto v = ex->getValue();
	auto t = v->type();

	if ( t->nameStr() == "string") {
		string s = ex->getValue()->asString();
		add(make_shared<UTF8>(s));
		add(make_shared<JString>( s ));
	}
	else if ( t->nameStr() == "int" || t->nameStr() == "char" ) {
		auto intv = static_pointer_cast<lang::TypedValue<int>, lang::Value>( v );
		int i = intv->value();
		add(make_shared<JInteger>(i));
	}
}

} /* namespace std */
