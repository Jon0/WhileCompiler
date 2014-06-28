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

string JavaDescriptor(shared_ptr<Type>);
string JavaDescriptor(shared_ptr<Func>);

class ConstantPool: public enable_shared_from_this<ConstantPool> {
public:
	ConstantPool();
	virtual ~ConstantPool();

	short size();

	/*
	 * add if not available, returns index
	 */
	short use(shared_ptr<Constant>);

	void add(shared_ptr<Constant>);
	short find(shared_ptr<Constant>);
	bool contains(shared_ptr<Constant>);

	bytecode getByteCode();

	string debug();


	// TODO unused or should make private?
	short lookup(int);
	short lookup(string);
	short lookupType(string, string);

	void accept(shared_ptr<Func>);

	void accept(shared_ptr<ConstExpr>);


private:
	vector<shared_ptr<Constant>> constant_pool;
	map<string, int> table;

};

} /* namespace std */

#endif /* CONSTANTPOOL_H_ */
