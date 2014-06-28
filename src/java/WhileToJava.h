/*
 * Bytecode.h
 *
 *  Created on: 19/04/2014
 *      Author: asdf
 */

#ifndef WHILETOJAVA_H_
#define WHILETOJAVA_H_

#include <memory>
#include <vector>

#include "../lang/SyntaxVisitor.h"

#include "Classfile.h"
#include "ClassfileWriter.h"

namespace std {

class Instruction {
public:
	vector<unsigned char> bytes;

	void modifyArg1(unsigned char c) {
		bytes[1] = c;
	}

	void modifyArg2(unsigned short c) {
		bytes[1] = (unsigned char) (c >> 8);
		bytes[2] = (unsigned char) (c >> 0);
	}

	void modifyArg4(unsigned int c) {
		bytes[1] = (unsigned char) (c >> 24);
		bytes[2] = (unsigned char) (c >> 16);
		bytes[3] = (unsigned char) (c >> 8);
		bytes[4] = (unsigned char) (c >> 0);
	}
};

class WhileToJava: public SyntaxVisitor, public enable_shared_from_this<WhileToJava> {
public:
	WhileToJava(ClassfileWriter &, ConstantPool &);
	virtual ~WhileToJava();

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
	virtual void accept(shared_ptr<EquivOp>);
	virtual void accept(shared_ptr<NotEquivOp>);
	virtual void accept(shared_ptr<AndExpr>);
	virtual void accept(shared_ptr<OrExpr>);
	virtual void accept(shared_ptr<NotExpr>);

private:
	ClassfileWriter &out;
	ConstantPool &constant_pool;

	// TODO remove
	vector<Instruction> istack;
	int num_locals;
	map<string, int> local_map;
	vector<shared_ptr<Type>> local_type;
	shared_ptr<Type> t_const; // type of the last constant visited;


	int stackSize();

	void addInstruction(unsigned char code);
	void addInstruction1(unsigned char code, unsigned char arg);
	void addInstruction2(unsigned char code, unsigned short arg);
	void addInstruction4(unsigned char code, unsigned int arg);
};

} /* namespace std */

#endif /* WHILETOJAVA_H_ */
