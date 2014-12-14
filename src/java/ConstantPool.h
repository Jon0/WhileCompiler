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
#include "JavaInstruction.h"

namespace std {

class Constant;
class Type;
class Func;

string JavaDescriptor(shared_ptr<lang::Type>);
string JavaDescriptor(shared_ptr<lang::Func>);

class ConstantPool: public enable_shared_from_this<ConstantPool> {
public:
	ConstantPool();
	virtual ~ConstantPool();

	short size();

	/*
	 * add if not available, returns index 
	 * of the existing or added constant
	 */
	short use(shared_ptr<Constant>);

	/**
	 * add without any checks
	 */
	void add(shared_ptr<Constant>);

	/**
	 * find index, return negative when not found
	 */
	short find(shared_ptr<Constant>);

	/**
	 * true if constant is contained
	 */
	bool contains(shared_ptr<Constant>);

	bytecode getByteCode();

	string debug();


	// TODO unused or should make private?
	short lookup(int);
	short lookup(string);
	short lookupType(string, string);

	void accept(shared_ptr<lang::Func>);

	void accept(shared_ptr<lang::ConstExpr>);


private:
	vector<shared_ptr<Constant>> constant_pool;
	map<string, int> table;

};

} /* namespace std */

#endif /* CONSTANTPOOL_H_ */
