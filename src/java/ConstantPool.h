/*
 * ConstantPool.h
 *
 *  Created on: 9/05/2014
 *      Author: remnanjona
 */

#ifndef CONSTANTPOOL_H_
#define CONSTANTPOOL_H_

#include <map>
#include <memory>
#include <vector>

#include "../lang/SyntaxVisitor.h"

#include "ClassfileWriter.h"

namespace std {

class Constant {
public:
	virtual ~Constant() {}
	virtual void writeByteCode(ClassfileWriter &) = 0;
	virtual string lookupStr() = 0;
	virtual string typeStr() = 0;
};

class UTF8: public Constant {
public:
	UTF8(string c) {
		str = c;
	}

	virtual void writeByteCode(ClassfileWriter &);
	virtual string lookupStr();
	virtual string typeStr();

private:
	string str;
};

class JClass: public Constant {
public:
	JClass(int c) {
		ind = c;
	}

	virtual void writeByteCode(ClassfileWriter &);
	virtual string lookupStr();
	virtual string typeStr();

private:
	int ind;
};

class JNameType: public Constant {
public:
	JNameType(int n, int d) {
		name_index = n;
		descriptor_index  = d;
	}

	virtual void writeByteCode(ClassfileWriter &);
	virtual string lookupStr();
	virtual string typeStr();

private:
	int name_index, descriptor_index;
};

class JFieldRef: public Constant {
public:
	JFieldRef(int n, int d) {
		class_index = n;
		nametype_index  = d;
	}

	virtual void writeByteCode(ClassfileWriter &);
	virtual string lookupStr();
	virtual string typeStr();

private:
	int class_index, nametype_index;
};

class JMethodRef: public Constant {
public:
	JMethodRef(int n, int d) {
		class_index = n;
		nametype_index  = d;
	}

	virtual void writeByteCode(ClassfileWriter &);
	virtual string lookupStr();
	virtual string typeStr();

private:
	int class_index, nametype_index;
};

class JString: public Constant {
public:
	JString(int n) {
		utf8_index = n;
	}

	virtual void writeByteCode(ClassfileWriter &);
	virtual string lookupStr();
	virtual string typeStr();

	int utf8Index();

private:
	int utf8_index;
};

class ConstantPool: public SyntaxVisitor, public enable_shared_from_this<ConstantPool> {
public:
	ConstantPool();
	virtual ~ConstantPool();

	void writeByteCode(ClassfileWriter &);
	void add(shared_ptr<Constant>);
	short lookup(string);
	short lookupType(string, string);
	short size();

	virtual void accept(shared_ptr<Type>);
	virtual void accept(shared_ptr<Value>);
	virtual void accept(shared_ptr<Func>);
	virtual void accept(shared_ptr<Program>);

	virtual void accept(shared_ptr<BlockStmt>);
	virtual void accept(shared_ptr<InitStmt>);
	virtual void accept(shared_ptr<AssignStmt>);
	virtual void accept(shared_ptr<IfStmt>);
	virtual void accept(shared_ptr<WhileStmt>);
	virtual void accept(shared_ptr<ForStmt>);
	virtual void accept(shared_ptr<PrintStmt>);
	virtual void accept(shared_ptr<EvalStmt>);
	virtual void accept(shared_ptr<ReturnStmt>);
	virtual void accept(shared_ptr<BreakStmt>);
	virtual void accept(shared_ptr<SwitchStmt>);

	virtual void accept(shared_ptr<ConstExpr>);
	virtual void accept(shared_ptr<IsTypeExpr>);
	virtual void accept(shared_ptr<VariableExpr>);
	virtual void accept(shared_ptr<FuncCallExpr>);
	virtual void accept(shared_ptr<RecordExpr>);

	virtual void accept(shared_ptr<ListExpr>);
	virtual void accept(shared_ptr<ListLengthExpr>);
	virtual void accept(shared_ptr<ConcatExpr>);
	virtual void accept(shared_ptr<ListLookupExpr>);
	virtual void accept(shared_ptr<RecordMemberExpr>);
	virtual void accept(shared_ptr<BasicCastExpr>);
	virtual void accept(shared_ptr<AbstractOpExpr>);

private:
	vector<shared_ptr<Constant>> constant_pool;
	map<string, int> table;

};

} /* namespace std */

#endif /* CONSTANTPOOL_H_ */
